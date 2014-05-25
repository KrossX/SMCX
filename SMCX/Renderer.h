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

class RENDERER
{
public:
	virtual void SetPixel(u16 x, u16 y, u8 pix) = 0;
	virtual void UpdatePalette(u32 *pal) = 0;

	virtual void RenderClear(bool reset_palette) = 0;
	virtual void RenderFrame() = 0;

	virtual void Shutdown() = 0;
	virtual void Restart(HWND hWnd) = 0;
	virtual HRESULT Init(HWND hWnd) = 0;
};


#endif