/* Copyright (c) 2012 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "main.h"
#include "machine.h"
#include "renderer.h"

extern Renderer *render;

void Machine::set_pixel(u16 x, u16 y, u8 pix)
{
	if(Mode_Mega)
		render->set_pixel(x, y, pix);
	else
	{
		for(u8 i = 0; i < ScaleX; i++)
		for(u8 j = 0; j < ScaleY; j++)
		{
			Display[x+i][y+j] = pix;
			render->set_pixel(x+i, y+j, pix);
		}
	}
}

void Machine::render_sprite()
{
	bool msmooth = Mode_Mega && mega_smooth;

	if(!msmooth)
		render->render_frame();
}

void Machine::render_clear()
{
	bool msmooth = Mode_Mega && mega_smooth;

	if(msmooth)
		render->render_frame();

	render->render_clear(!Mode_Mega);
}

void Machine::load_palette()
{
	//ARGB to d3d BGRA

	render->set_palette(Palette);
}