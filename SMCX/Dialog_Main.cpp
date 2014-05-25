/*  SMCX - SuperMegaChip-X Emulator
 *  Copyright (C) 2012  KrossX
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "Main.h"
#include "Dialog.h"
#include "resource.h"
#include "Machine.h"
#include "Renderer.h"

#include <Uxtheme.h>
#include <CommCtrl.h>
#include <cstdio>
#include <math.h>

wchar_t windowTitle[] = L"SuperMegaChip-X";
wchar_t windowClass[] = L"SUPERMEGACHIPX";

extern MACHINE * machine;
extern RENDERER * render;

extern HINSTANCE hInst;
extern bool romloaded;
extern bool stopped;

HWND hConfig = NULL;

void MenuRunSet(HWND hWnd, bool enable)
{
	HMENU menu = GetMenu(hWnd);
	EnableMenuItem(menu, ID_RUN_START, enable ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(menu, ID_RUN_RESET, enable ? MF_ENABLED : MF_GRAYED);
}

INT_PTR CALLBACK ConfigProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		{
			SendMessage(GetDlgItem(hWnd, IDC_CPU_SLIDER), TBM_SETRANGE, TRUE, MAKELONG(5, 301));
			SendMessage(GetDlgItem(hWnd, IDC_GPU_SLIDER), TBM_SETRANGE, TRUE, MAKELONG(5, 61));

			s32 cpuvalue = (s32)sqrt(machine->cpuFreq * 1.0);
			cpuvalue = cpuvalue > 300 ? 0 : cpuvalue < 5 ? 0 : cpuvalue;

			SendMessage(GetDlgItem(hWnd, IDC_CPU_SLIDER), TBM_SETPOS, TRUE, cpuvalue);
			SendMessage(GetDlgItem(hWnd, IDC_GPU_SLIDER), TBM_SETPOS, TRUE, machine->gpuFreq);

			SendMessage(hWnd, WM_HSCROLL, 0, (LPARAM)GetDlgItem(hWnd, IDC_CPU_SLIDER));
			SendMessage(hWnd, WM_HSCROLL, 0, (LPARAM)GetDlgItem(hWnd, IDC_GPU_SLIDER));

			CheckDlgButton(hWnd, IDC_MEGASMOOTH, machine->Mega_Smooth ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_VSYNC, machine->vsync ? BST_CHECKED : BST_UNCHECKED);

			ShowWindow(hWnd, SW_SHOW);
		}
		break;

	case WM_HSCROLL:
		{
			if((HWND)lParam == GetDlgItem(hWnd, IDC_CPU_SLIDER))
			{
				wchar_t text[80] = {0};

				s32 value = SendMessage(GetDlgItem(hWnd, IDC_CPU_SLIDER),TBM_GETPOS,0,0);

				value = value > 300 ? 0 : value;

				if(value == 0)
					swprintf(text, L"MAX");
				else 
				{
					value *= value;

					if(value < 1000)
						swprintf(text, L"%2.1f Hz", value * 1.0f);
					else
						swprintf(text, L"%2.1f KHz", value * 0.001f);
				}
			
				SetDlgItemText(hWnd, IDC_CPU_VALUE, text);

				machine->SetCPUfreq(value);
			}
			else  if((HWND)lParam == GetDlgItem(hWnd, IDC_GPU_SLIDER))
			{
				wchar_t text[80] = {0};

				s32 value = SendMessage(GetDlgItem(hWnd, IDC_GPU_SLIDER),TBM_GETPOS,0,0);

				value = value > 60 ? 0 : value;
				swprintf(text, value < 10 ? L"MAX" : L"%d", value);
				SetDlgItemText(hWnd, IDC_GPU_VALUE, text);

				machine->SetGPUfreq(value);
			}
		} break;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_MEGASMOOTH:
				machine->Mega_Smooth = IsDlgButtonChecked(hWnd, IDC_MEGASMOOTH) == BST_CHECKED? true : false;
				break;

			case IDC_VSYNC:
				machine->vsync = IsDlgButtonChecked(hWnd, IDC_VSYNC) == BST_CHECKED? 1 : 0;
				break;
			}
		}
		break;

	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYUP:
		machine->UpdateInput(wParam & 0xFF, 0);
		break;

	case WM_KEYDOWN:
		switch(wParam) // For hotkeys...
		{
		case VK_F1: // Open 
			SendMessage(hWnd, WM_COMMAND, ID_FILE_OPENROM, NULL); 
			break; 

		case VK_F2: // Start/Stop
			if(romloaded)
				SendMessage(hWnd, WM_COMMAND, ID_RUN_START, NULL); 
			break; 

		case VK_F3: // Reset
			if(romloaded)
				SendMessage(hWnd, WM_COMMAND, ID_RUN_RESET, NULL); 
			break; 

		case VK_F4: // Config
			SendMessage(hWnd, WM_COMMAND, ID_RUN_CONFIG, NULL); 
			break; 

		case VK_ESCAPE:
			if(hConfig)
			{
				EndDialog(hConfig, 0);
				hConfig = NULL;
			}
			else
				PostQuitMessage(0);
			break;

		default:
			machine->UpdateInput(wParam & 0xFF, 1);
		}
		break;

	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)wParam;
			wchar_t filename[MAX_PATH] = {0};

			if(DragQueryFile(hDrop, 0, filename, MAX_PATH) &&
				machine->LoadROM(filename))
			{
				MenuRunSet(hWnd, true);
				romloaded = true;
				stopped = false;
				machine->Reset();
			}
			DragFinish(hDrop);
		}
		break;

	case WM_COMMAND:
		{
			u16 wmId    = LOWORD(wParam);
			u16 wmEvent = HIWORD(wParam);

			switch (wmId)
			{
			case ID_RUN_START:
				stopped = !stopped;
				break;

			case ID_RUN_RESET:
				machine->Reset();
				break;

			case ID_RUN_CONFIG:
				hConfig = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CONFIG), hWnd, ConfigProc);
				break;

			case ID_FILE_OPENROM:
				{
					OPENFILENAME opendialog = {0};
					wchar_t filename[MAX_PATH] = {0};
					wchar_t directory[MAX_PATH] = {0};

					GetCurrentDirectory(MAX_PATH, directory);

					opendialog.lStructSize = sizeof(opendialog);
					opendialog.hwndOwner = hWnd;
					opendialog.hInstance = hInst;
					opendialog.lpstrFile = filename;
					opendialog.nMaxFile = MAX_PATH;
					opendialog.lpstrInitialDir = directory;
					opendialog.lpstrTitle = L"Open ROM";
					opendialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if(GetOpenFileName(&opendialog) && machine->LoadROM(filename))
					{
						MenuRunSet(hWnd, true);
						romloaded = true;
						stopped = false;
						machine->Reset();
					}
				}
				break;

			case ID_FILE_EXIT:
				PostQuitMessage(0);
				break;
			}
		}
		break;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_SIZE:
		render->Restart(hWnd);
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	// WM_CLOSE
	// WM_QUIT
	// WM_DESTROY

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return FALSE;
}

ATOM DialogRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMCX));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDM_SMCX);
	wcex.lpszClassName	= windowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMCX));

	return RegisterClassEx(&wcex);
}

BOOL InitDialog(HINSTANCE hInstance, int nCmdShow, HWND &hWnd)
{
	DialogRegisterClass(hInstance);

	hWnd = CreateWindow(windowClass, windowTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 640, 480, NULL, NULL, hInstance, NULL);

	if (!hWnd) return FALSE;

	DragAcceptFiles(hWnd, TRUE);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}