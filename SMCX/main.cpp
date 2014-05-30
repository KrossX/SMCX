/* Copyright (c) 2012 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
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



