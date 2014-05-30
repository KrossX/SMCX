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

#ifndef RENDERER_H
#define RENDERER_H

class Renderer
{
public:
	virtual void set_pixel(u16 x, u16 y, u8 pix) = 0;
	virtual void set_palette(u32 *pal) = 0;

	virtual void render_clear(bool reset_palette) = 0;
	virtual void render_frame() = 0;

	virtual void shutdown() = 0;
	virtual void restart(HWND hwnd) = 0;
	virtual HRESULT init(HWND hwnd) = 0;
};


#endif