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

#include "main.h"
#include "machine.h"
#include "dialog.h"
#include "renderer.h"
#include "renderer_ogl.h"

HINSTANCE hinst = NULL;

Machine chip;
Renderer_OGL render_ogl;
Renderer* render = NULL;

bool romloaded = false;
bool stopped = true;

int shutdown()
{
	if(render) render->shutdown();
	chip.shutdown();
	return 0;
}

int CALLBACK wWinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPWSTR lpcmdline, int ncmdshow)
{
	HWND hwnd = NULL;
	hinst = hinstance;

	SetProcessAffinityMask(GetCurrentProcess(), 1);

	render = &render_ogl;
	
	if (!InitDialog(hinstance, ncmdshow, hwnd) ||
		render->init(hwnd) != S_OK)
	{
		return shutdown();
	}

	chip.start();

	if (chip.load_rom(lpcmdline))
	{
		romloaded = true;
		stopped = false;
		MenuRunSet(hwnd, true);
		chip.reset();
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
			chip.loop();

		Sleep(0);
	}

	return shutdown();
}



