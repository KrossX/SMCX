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

u8 Keyboard[256] = {0};

void MACHINE::UpdateInput(u8 key, u8 state)
{
	//Sleep(1);

	switch(key)
	{
	case 0x58: Input[0x0] = state; break; // X
	case 0x31: Input[0x1] = state; break; // 1
	case 0x32: Input[0x2] = state; break; // 2
	case 0x33: Input[0x3] = state; break; // 3
	case 0x51: Input[0x4] = state; break; // Q
	case 0x57: Input[0x5] = state; break; // W
	case 0x45: Input[0x6] = state; break; // E
	case 0x41: Input[0x7] = state; break; // A
	case 0x53: Input[0x8] = state; break; // S
	case 0x44: Input[0x9] = state; break; // D
	case 0x5A: Input[0xA] = state; break; // Z
	case 0x43: Input[0xB] = state; break; // C
	case 0x34: Input[0xC] = state; break; // 4
	case 0x52: Input[0xD] = state; break; // R
	case 0x46: Input[0xE] = state; break; // F
	case 0x56: Input[0xF] = state; break; // V
	}
};

/* 
FROM

1	2	3	C
4	5	6	D
7	8	9	E
A	0	B	F

TO

1	2	3	4
Q	W	E	R
A	S	D	F
Z	X	C	V
*/