/* Copyright (c) 2016 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "smcx.h"

#include "video.h"
#include "sound.h"

#include "chip8.h"
#include "chip16.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HINSTANCE instance;
HWND      window;

char wnd_class[] = "SMCX_CLASS";
char wnd_title[] = "SCMX";
int  wnd_width;
int  wnd_height;

double timer_freq;
double timer_curr;

HANDLE video_thread;
HANDLE chip8_thread;
HANDLE chip16_thread;

int video_run;
int chip8_run;
int chip16_run;

void chip8_start(void);
void chip8_stop(void);
void chip16_start(void);
void chip16_stop(void);

BOOL open_dialog(char *filename)
{
	OPENFILENAME open;
	char dir[MAX_PATH];

	memset(&open, 0, sizeof(OPENFILENAME));
	memset(dir, 0, MAX_PATH);

	GetCurrentDirectory(MAX_PATH, dir);

	open.lStructSize = sizeof(OPENFILENAME);
	open.hwndOwner = window;
	open.hInstance = instance;
	open.lpstrFile = filename;
	open.nMaxFile = MAX_PATH;
	open.lpstrInitialDir = dir;
	open.lpstrTitle = "Open ROM";
	open.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	return GetOpenFileName(&open);
}

void load_file(char *filename, void **buffer, long *buffsize)
{
	FILE *rom = fopen(filename, "rb");

	if(rom)
	{
		long size = 0;

		fseek(rom, 0, SEEK_END);
		size = ftell(rom);

		//if(size > 0 && size <= 0xE00)
		if(size > 0 && size <= 0xFFFE00) // MEGA
		{
			*buffer = (char*)malloc(size);

			fseek(rom, 0, SEEK_SET);
			fread(*buffer, 1, size, rom);
			*buffsize = size;
		}

		fclose(rom);
	}
}

int is_chip16_rom(char *filename)
{
	size_t len = strlen(filename);

	if(len > 4)
	{
		char *f = &filename[len - 4];

		if(f[1] == 'C') f[1] = 'c';
		if(strcmp(".c16", f) == 0)
			return 1;
	}

	return 0;
}

LRESULT CALLBACK wnd_proc(HWND wnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
	switch (msg)
	{
	case WM_CREATE:
		//ShowCursor(FALSE);
		ShowWindow(wnd, SW_SHOW);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		wnd_width = LOWORD(l_param);
		wnd_height = HIWORD(l_param);
		video_window_update(wnd_width, wnd_height);
		break;

	case WM_KEYDOWN:
		chip8_input(w_param, 1);
		chip16_input(w_param, 1);
		break;

	case WM_KEYUP:
		chip8_input(w_param, 0);
		chip16_input(w_param, 0);
		break;

	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)w_param;
			char filename[MAX_PATH];

			DragQueryFileA(hDrop, 0, filename, MAX_PATH);
			DragFinish(hDrop);

			//if(open_dialog(filename))
			{
				void *buffer = NULL;
				long size = 0;

				load_file(filename, &buffer, &size);

				if(buffer)
				{
					chip8_stop();
					chip16_stop();

					if(is_chip16_rom(filename))
					{
						chip16_load(buffer, size);
						chip16_start();
					}
					else
					{
						chip8_load(buffer, size);
						chip8_start();
					}

					free(buffer);
					//SetForegroundWindow(window);
				}
			}	
		}
		break;
		
	//case WM_SIZING: break;

	case MM_WOM_OPEN:
	case MM_WOM_CLOSE:
	case MM_WOM_DONE:
		 snd_proc(msg, (void*)l_param);
		 break;
	}

	return DefWindowProc(wnd, msg, w_param, l_param);
}

int create_window(HINSTANCE inst)
{
	WNDCLASS wc;
	ATOM rc;
	DWORD style, stylex;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = wnd_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = inst;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = wnd_class;

	rc = RegisterClass(&wc);
	if(rc == 0) return 1;

	style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS; // ogl required?
	stylex = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	
	window = CreateWindowEx(stylex, wnd_class, wnd_title, style,
		CW_USEDEFAULT, CW_USEDEFAULT, wnd_width, wnd_height, NULL, NULL, inst, NULL);
	if(window == 0) return 2;

	return ERROR_SUCCESS;
}

void timer_getfreq(void)
{
	LARGE_INTEGER perf_freq;
	QueryPerformanceFrequency(&perf_freq);
	timer_freq = (double)perf_freq.QuadPart;
}

void timer_getcurr(void)
{
	LARGE_INTEGER perf_count;
	QueryPerformanceCounter(&perf_count);
	timer_curr = (double)perf_count.QuadPart;
}


DWORD WINAPI chip8(LPVOID param)
{
	chip8_init();

	video_disp_buffer(256, 192, chip8_getdisplay());
	video_disp_res(64, 32);

	while(chip8_run)
	{
		timer_getcurr();
		chip8_loop(timer_curr/timer_freq);
		Sleep(0);
	}

	chip8_shutdown();

	return ERROR_SUCCESS;
}

DWORD WINAPI chip16(LPVOID param)
{
	chip16_init();

	video_disp_buffer(320, 240, chip16_getdisplay());
	video_disp_res(320, 240);

	while(chip16_run)
	{
		timer_getcurr();
		chip16_loop(timer_curr/timer_freq);
		Sleep(0);
	}

	chip16_shutdown();

	return ERROR_SUCCESS;
}

DWORD WINAPI video(LPVOID param)
{
	video_prepare();

	while(video_run)
	{
		video_loop();
		Sleep(0);
	}

	video_shutdown();
	
	return ERROR_SUCCESS;
}

void chip8_start(void)
{
	chip8_run = 1;
	chip8_thread =  CreateThread(NULL, 0, chip8, NULL, 0, NULL);
}

void chip8_stop(void)
{
	chip8_run = 0;
	WaitForSingleObject(chip8_thread, INFINITE);
	CloseHandle(chip8_thread);
	chip8_thread = NULL;
}

void chip16_start(void)
{
	chip16_run = 1;
	chip16_thread =  CreateThread(NULL, 0, chip16, NULL, 0, NULL);
}

void chip16_stop(void)
{
	chip16_run = 0;
	WaitForSingleObject(chip16_thread, INFINITE);
	CloseHandle(chip16_thread);
	chip16_thread = NULL;
}
 
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmdline, int cmdshow)
{
	MSG msg;

	instance   = inst;
	wnd_width  = 640;
	wnd_height = 480;

	if(create_window(inst)) return 1;
	if(video_init(window))  return 2;
	if(snd_init(window))    return 3;

	DragAcceptFiles(window, TRUE);
	timer_getfreq();

	video_run = 1;
	video_thread = CreateThread(NULL, 0, video, NULL, 0, NULL);

	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(chip8_run) chip8_stop();
	if(chip16_run) chip16_stop();

	snd_stop();
	snd_shutdown();

	video_run = 0;
	WaitForSingleObject(video_thread, INFINITE);
	CloseHandle(video_thread);

	return 0;
}



