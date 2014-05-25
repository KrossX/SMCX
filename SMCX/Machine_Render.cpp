﻿/*  SMCX - SuperMegaChip-X Emulator
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
#include "Renderer.h"

extern RENDERER *render;

void MACHINE::SetPixel(u16 x, u16 y, u8 pix)
{
	if(Mode_Mega)
		render->SetPixel(x, y, pix);
	else
	{
		for(u8 i = 0; i < ScaleX; i++)
		for(u8 j = 0; j < ScaleY; j++)
		{
			Display[x+i][y+j] = pix;
			render->SetPixel(x+i, y+j, pix);
		}
	}
}

void MACHINE::RenderSprite()
{
	bool msmooth = Mode_Mega && Mega_Smooth;

	if(!msmooth)
		render->RenderFrame();
}

void MACHINE::RenderClear()
{
	bool msmooth = Mode_Mega && Mega_Smooth;

	if(msmooth)
		render->RenderFrame();

	render->RenderClear(!Mode_Mega);
}

void MACHINE::LoadPalette()
{
	//ARGB to d3d BGRA

	render->UpdatePalette(Palette);
}