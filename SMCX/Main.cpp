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
#include "Machine.h"
#include "Dialog.h"
#include "Renderer.h"

HINSTANCE hInst = NULL;
Machine * machine = NULL;
Render * render = NULL;

bool romloaded = false;
bool stopped = true;

int Shutdown()
{
	if(render) render->Shutdown();
	if(machine) machine->Shutdown();
	return 0;
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd = NULL;
	hInst = hInstance;

	SetProcessAffinityMask(GetCurrentProcess(), 1);

	if(!InitDialog(hInst, nCmdShow, hWnd)) return Shutdown();
	
	render = new Render();
	if(!render || render->Init(hWnd) != S_OK) return Shutdown();

	machine = new Machine();
	if(!machine) return Shutdown();

	machine->Start();

	if(machine->LoadROM(lpCmdLine))
	{
		romloaded = true;
		stopped = false;
		MenuRunSet(hWnd, true);
		machine->Reset();
	}

	MSG msg;
	bool run = true;
	
	while (run)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(msg.message == WM_QUIT) run = false;
		}

		if(romloaded && !stopped)
			machine->Loop();

		Sleep(0);
	}

	return Shutdown();
}



