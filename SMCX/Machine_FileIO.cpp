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
#include <fstream>

u8 Font[] = {0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			 0x20, 0x60, 0x20, 0x20, 0x70, // 1
			 0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			 0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			 0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			 0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			 0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			 0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			 0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			 0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			 0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			 0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			 0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			 0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			 0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			 0xF0, 0x80, 0xF0, 0x80, 0x80};// F

u16 Font10[] = 
			{0x90F0, 0x9090, 0x9090, 0x9090, 0x00F0, // 0
			 0x2020, 0x2060, 0x2020, 0x2020, 0x0070, // 1
			 0x10F0, 0x1010, 0x80F0, 0x8080, 0x00F0, // 2
			 0x10F0, 0x1010, 0x10F0, 0x1010, 0x00F0, // 3
			 0x9090, 0x9090, 0x10F0, 0x1010, 0x0010, // 4
			 0x80F0, 0x8080, 0x10F0, 0x1010, 0x00F0, // 5
			 0x80F0, 0x8080, 0x90F0, 0x9090, 0x00F0, // 6
			 0x10F0, 0x2010, 0x4020, 0x4040, 0x0040, // 7
			 0x90F0, 0x9090, 0x90F0, 0x9090, 0x00F0, // 8
			 0x90F0, 0x9090, 0x10F0, 0x1010, 0x00F0, // 9
			 0x90F0, 0x9090, 0x90F0, 0x9090, 0x0090, // A
			 0x90E0, 0x9090, 0x90E0, 0x9090, 0x00E0, // B
			 0x80F0, 0x8080, 0x8080, 0x8080, 0x00F0, // C
			 0x90E0, 0x9090, 0x9090, 0x9090, 0x00E0, // D
			 0x80F0, 0x8080, 0x80F0, 0x8080, 0x00F0, // E
			 0x80F0, 0x8080, 0x80F0, 0x8080, 0x0080};// F


bool MACHINE::LoadROM(const wchar_t * name)
{
	memset(Memory, 0, sizeof(Memory));
	memcpy(Memory, Font, sizeof(Font));
	memcpy(Memory + 80, Font10, sizeof(Font10));

	bool loaded = false;
	if(!name || name[0] == 0) return loaded;

	std::wstring filename(name);
	if(name[0] == L'"') filename = filename.substr(1, filename.find_last_of(L'"') - 1);
	std::fstream rom(filename, std::ios_base::in | std::ios_base::binary);

	if(rom.is_open())
	{
		rom.seekg(0, std::ios::end);
		std::streamsize size = rom.tellg();

		if(size < 0x2000000 /*4096 for CHIP8*/)
		{
			rom.seekg(0, std::ios::beg);
			rom.read((char*)&Memory[0x200], size);
			loaded = true;
		}
		else
			loaded = false;

		rom.close();
	}

	return loaded;
}