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

/*
 * CHIP8
 */

void Machine::_0NNN() //Calls RCA 1802 program at address NNN.
{
	DebugLineOut(opcode.RAW, "call\t%3X RCA 1802", opcode.RAW);
	ProfilerUpdate(0);
	// Ignored?
}

void Machine::_00E0() //Clears the screen.
{
	DebugLineOut(opcode.RAW, "cls");
	ProfilerUpdate(1);

	memset(Display, 0, sizeof(Display));
	RenderClear(); 
}

void Machine::_00EE() //Returns from a subroutine.
{
	DebugLineOut(opcode.RAW, "ret");
	ProfilerUpdate(2);

	PC = Stack[SP--];
}

void Machine::_1NNN() //Jumps to address NNN.
{
	DebugLineOut(opcode.RAW, "jmp\t%3X", opcode.RAW & 0xFFF);
	ProfilerUpdate(3);
	
	PC = opcode.RAW & 0xFFF;
}

void Machine::_2NNN() //Calls subroutine at NNN.
{
	DebugLineOut(opcode.RAW, "call\t%3X", opcode.RAW & 0xFFF);
	ProfilerUpdate(4);

	Stack[++SP] = PC;
	PC = opcode.RAW & 0xFFF;
}

void Machine::_3XNN() //Skips the next instruction if VX equals NN.
{
	DebugLineOut(opcode.RAW, "skip\tV%X == %2X", opcode.X, opcode.NN);
	ProfilerUpdate(5);

	if(V[opcode.X] == opcode.NN) PC += 2;
}

void Machine::_4XNN() //Skips the next instruction if VX doesn't equal NN.
{
	DebugLineOut(opcode.RAW, "skip\tV%X != %2X", opcode.X, opcode.NN);
	ProfilerUpdate(6);

	if(V[opcode.X] != opcode.NN) PC += 2;
}

void Machine::_5XY0() //Skips the next instruction if VX equals VY.
{
	DebugLineOut(opcode.RAW, "skip\tV%X == V%X", opcode.X, opcode.Y);
	ProfilerUpdate(7);

	if(V[opcode.X] == V[opcode.Y]) PC += 2;
}

void Machine::_6XNN() //Sets VX to NN.
{
	DebugLineOut(opcode.RAW, "set\tV%X <- %2X", opcode.X, opcode.NN);
	ProfilerUpdate(8);

	V[opcode.X] = opcode.NN;
}

void Machine::_7XNN() //Adds NN to VX.
{
	DebugLineOut(opcode.RAW, "add\tV%X <- %2X", opcode.X, opcode.NN);
	ProfilerUpdate(9);

	V[opcode.X] = (V[opcode.X] + opcode.NN) & 0xFF;
}

void Machine::_8XY0() //Sets VX to the value of VY.
{
	DebugLineOut(opcode.RAW, "set\tV%X <- V%X", opcode.X, opcode.Y);
	ProfilerUpdate(10);

	V[opcode.X] = V[opcode.Y];
}

void Machine::_8XY1() //Sets VX to VX or VY.
{
	DebugLineOut(opcode.RAW, "set\tV%X |= V%X", opcode.X, opcode.Y);
	ProfilerUpdate(11);

	V[opcode.X] |= V[opcode.Y];
}

void Machine::_8XY2() //Sets VX to VX and VY.
{
	DebugLineOut(opcode.RAW, "set\tV%X &= V%X", opcode.X, opcode.Y);
	ProfilerUpdate(12);

	V[opcode.X] &= V[opcode.Y];
}

void Machine::_8XY3() //Sets VX to VX xor VY.
{
	DebugLineOut(opcode.RAW, "set\tV%X ^= V%X", opcode.X, opcode.Y);
	ProfilerUpdate(13);

	V[opcode.X] ^= V[opcode.Y];
}

void Machine::_8XY4() //Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
{
	DebugLineOut(opcode.RAW, "add\tV%X <- V%X /F", opcode.X, opcode.Y);
	ProfilerUpdate(14);

	const u8 x = opcode.X;
	const u8 y = opcode.Y;
	
	const u32 value = V[x] + V[y];
	V[0xF] = value > 0xFF? 1 : 0;
	V[x] = value & 0xFF;
}

void Machine::_8XY5() //VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
{
	DebugLineOut(opcode.RAW, "add\tV%X <- -V%X /F", opcode.X, opcode.Y);
	ProfilerUpdate(15);

	const u8 x = opcode.X;
	const u8 y = opcode.Y;
	
	V[0xF] = V[x] >= V[y]? 1 : 0; // >= ??
	V[x] = (V[x] - V[y]) & 0xFF;
}

void Machine::_8XY6() //Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
{
	DebugLineOut(opcode.RAW, "shr\tV%X /F", opcode.X);
	ProfilerUpdate(16);

	const u8 x = opcode.X;
	
	V[0xF] = V[x] & 0x1;
	V[x] = V[x] >> 1;
}

void Machine::_8XY7() //Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
{
	DebugLineOut(opcode.RAW, "set\tV%X <- V%X - V%X /F", opcode.X, opcode.Y, opcode.X);
	ProfilerUpdate(17);

	const u8 x = opcode.X;
	const u8 y = opcode.Y;
	
	V[0xF] = V[y] >= V[x]? 1 : 0; // >= ??
	V[x] = (V[y] - V[x]) & 0xFF;
}

void Machine::_8XYE() //Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
{
	DebugLineOut(opcode.RAW, "shl\tV%X /F", opcode.X);
	ProfilerUpdate(18);

	const u8 x = opcode.X;
	
	V[0xF] = V[x] & 0x80 ? 1 : 0;
	V[x] = (V[x] << 1) & 0xFF;
}

void Machine::_9XY0() //Skips the next instruction if VX doesn't equal VY.
{
	DebugLineOut(opcode.RAW, "skip\tV%X != V%X", opcode.X, opcode.Y);
	ProfilerUpdate(19);

	if(V[opcode.X] != V[opcode.Y]) PC += 2;
}

void Machine::_ANNN() //Sets I to the address NNN.
{
	DebugLineOut(opcode.RAW, "set\tI <- %3X", opcode.RAW & 0xFFF);
	ProfilerUpdate(20);

	I = Wrap<int>(opcode.RAW & 0xFFF, 0x200, 0xFFF);
}

void Machine::_BNNN() //Jumps to the address NNN plus V0.
{
	DebugLineOut(opcode.RAW, "jmp\t%3X + V0", opcode.RAW & 0xFFF);
	ProfilerUpdate(21);

	PC = opcode.RAW & 0xFFF + V[0];
}

void Machine::_CXNN() //Sets VX to a random number and NN.
{
	DebugLineOut(opcode.RAW, "set\tV%X <- Rand & %2X", opcode.X, opcode.NN);
	ProfilerUpdate(22);

	V[opcode.X] = (rand() % 0xFF) & opcode.NN;
}

//Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
// Each row of 8 pixels is read as bit-coded (with the most significant bit of each byte displayed
// on the left) starting from memory location I; I value doesn't change after the execution of 
// this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set 
// to unset when the sprite is drawn, and to 0 if that doesn't happen.
void Machine::_DXYN() 
{
	DebugLineOut(opcode.RAW, "draw\t(%d, %d, %d)", opcode.X, opcode.Y, opcode.N);
	ProfilerUpdate(23);

	const u8 ox = V[opcode.X];
	const u8 oy = V[opcode.Y];
	const u8 n = opcode.N == 0 ? 16 : opcode.N;
	const u8 w = n == 16 ? 16 : 8;

	V[0xF] = 0;

	u8 pixel;
	u8 *mem = &Memory[I];
	u16 byte = 0;
	u16 nx, ny;

	u8 mx = 256 / ScaleX - 1;
	u8 my = 192 / ScaleY - 1;

	for(u16 y = 0; y < n; y++)
	{
		if(Mode_SChip && n == 16)
		{
			byte  = *mem << 8; mem++;
			byte |= *mem; mem++;
		}
		else
		{
			byte = *mem; mem++;
		}

		for(u16 x = 0; x < w; x++)
		{
			nx = (ox + x) & mx; nx = (nx * ScaleX) % 256;
			ny = (oy + y) & my; ny = (ny * ScaleY) % 192;
			pixel = (byte >> (w-x-1)) & 0x1;

			if(!V[0xF] && Display[nx][ny] && pixel) V[0xF] = 1;

			Display[nx][ny] ^= pixel;
			SetPixel(nx, ny, Display[nx][ny]);
		}
	}

	//RenderSprite();
}

void Machine::_EX9E() //Skips the next instruction if the key stored in VX is pressed.
{
	DebugLineOut(opcode.RAW, "skip\tKey V%X", opcode.X);
	ProfilerUpdate(24);

	if(Input[V[opcode.X] & 0xF]) PC += 2;
}

void Machine::_EXA1() //Skips the next instruction if the key stored in VX isn't pressed.
{
	DebugLineOut(opcode.RAW, "skip\t!Key V%X", opcode.X);
	ProfilerUpdate(25);
	
	if(!Input[V[opcode.X] & 0xF]) PC += 2;
}

void Machine::_FX07() //Sets VX to the value of the delay timer.
{
	DebugLineOut(opcode.RAW, "set\tV%X <- DT", opcode.X);
	ProfilerUpdate(26);

	V[opcode.X] = DelayTimer;
}

void Machine::_FX0A() //A key press is awaited, and then stored in VX.
{
	DebugLineOut(opcode.RAW, "wait\tV%X <- Key", opcode.X);
	//ProfilerUpdate(27);

	for(u8 i = 0; i < 16; i++)
	{
		if(Input[i])
		{
			V[opcode.X] = i;
			Input[i] = 0;
			return;
		}
	}

	PC -= 2;
}

void Machine::_FX15() //Sets the delay timer to VX.
{
	DebugLineOut(opcode.RAW, "set\tDT <- V%X", opcode.X);
	ProfilerUpdate(28);

	DelayTimer = V[opcode.X];
}

void Machine::_FX18() //Sets the sound timer to VX.
{
	DebugLineOut(opcode.RAW, "set\tST <- V%X", opcode.X);
	ProfilerUpdate(29);

	SoundTimer = V[opcode.X];
}

void Machine::_FX1E() //Adds VX to I.
{
	DebugLineOut(opcode.RAW, "add\tI <- V%X", opcode.X);
	ProfilerUpdate(30);

	u16 value = I + V[opcode.X];
	if(value > 0xFFF) V[0xF] = 1; // Undocumented overflow flag
	
	I = Wrap<int>(value, 0x200, 0xFFF);
}

void Machine::_FX29() //Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
{
	DebugLineOut(opcode.RAW, "set\tI <- Char in V%X", opcode.X);
	ProfilerUpdate(31);

	I =  V[opcode.X] * 5;
}

void Machine::_FX33() //Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
{
	DebugLineOut(opcode.RAW, "set\tI <- Digits V%X", opcode.X);
	ProfilerUpdate(32);

	const u16 vx = V[opcode.X];
	const u8 hundreds = vx / 100;
	const u8 tens = ((vx - hundreds * 100) / 10);
	const u8 ones = vx - hundreds * 100 - tens * 10;

	Memory[I] = hundreds;
	Memory[I+1] = tens;
	Memory[I+2] = ones;
}

void Machine::_FX55() //Stores V0 to VX in memory starting at address I.
{
	DebugLineOut(opcode.RAW, "set\tI <- Regs V%X", opcode.X);
	ProfilerUpdate(33);

	const u8 x = opcode.X + 1;
	memcpy(&Memory[I], V, x);
}

void Machine::_FX65() //Fills V0 to VX with values from memory starting at address I.
{
	DebugLineOut(opcode.RAW, "set\tRegs V%X <- I", opcode.X);
	ProfilerUpdate(34);
	
	const u8 x = opcode.X + 1;
	memcpy(V, &Memory[I], x);
}

/*
 * CHIP8 - Hi Res
 */

void Machine::_H1260() // Turns on Hi-Res mode (64x64)
{
	DebugLineOut(opcode.RAW, "Hi-Res mode enabled");
	ProfilerUpdate(35);

	Mode_Chip8H = true;
	UpdateRect();
	PC = 0x2C0; // Jumps to 2C0 for some reason
}

void Machine::_H0230() // Hi-Res CLS
{
	DebugLineOut(opcode.RAW, "Hi-Res cls");
	ProfilerUpdate(36);

	_00E0();
}


/*
 * SUPER-CHIP8
 */

void Machine::_S00CN() // Scroll display N lines down
{
	DebugLineOut(opcode.RAW, "scroll down %d (SUPER)", opcode.N);
	ProfilerUpdate(37);

	bool oldMode = Mode_SChip;
	Mode_SChip = true;
	UpdateRect();

	u8 oldDisplay[256][192];
	memcpy(oldDisplay, Display, sizeof(Display));

	for(u16 x = 0; x < 256; x+=2) 
		for(u16 y = 0; y < 192; y+=3)
		{
			u16 ny = y - opcode.N * 3;

			if(ny < 0 || ny > 191)
				Display[x][y] = 0;
			else
				Display[x][y] = oldDisplay[x][ny];

			SetPixel(x, y, Display[x][y]);
		}

	Mode_SChip = oldMode;
	UpdateRect();

	//RenderSprite();
}

void Machine::_S00FB() // Scroll display 4 pixels right
{
	DebugLineOut(opcode.RAW, "scroll right 4 (SUPER)");
	ProfilerUpdate(38);

	bool oldMode = Mode_SChip;
	Mode_SChip = true;
	UpdateRect();

	u8 oldDisplay[256][192];
	memcpy(oldDisplay, Display, sizeof(Display));
		
	for(u16 x = 0; x < 256; x+=2) 
		for(u16 y = 0; y < 192; y+=3)
		{
			u16 nx =  x - 8;

			if(nx < 0 || nx > 255)
				Display[x][y] = 0;
			else
				Display[x][y] = oldDisplay[nx][y];

			SetPixel(x, y, Display[x][y]);
		}

	Mode_SChip = oldMode;
	UpdateRect();

	//RenderSprite();
}

void Machine::_S00FC() // Scroll display 4 pixels left
{
	DebugLineOut(opcode.RAW, "scroll left 4 (SUPER)");
	ProfilerUpdate(39);

	bool oldMode = Mode_SChip;
	Mode_SChip = true;
	UpdateRect();

	u8 oldDisplay[256][192];
	memcpy(oldDisplay, Display, sizeof(Display));
		
	for(u16 x = 0; x < 256; x+=2) 
		for(u16 y = 0; y < 192; y+=3)
		{
			u16 nx = x + 8;
			
			if(nx < 0 || nx > 255)
				Display[x][y] = 0;
			else
				Display[x][y] = oldDisplay[nx][y];

			SetPixel(x, y, Display[x][y]);
		}

	Mode_SChip = oldMode;
	UpdateRect();

	//RenderSprite();
}

void Machine::_S00FD() // Exit CHIP interpreter
{
	DebugLineOut(opcode.RAW, "exit CHIP interpreter (SUPER)");
	ProfilerUpdate(40);
}

void Machine::_S00FE() // Disable extended screen mode
{
	DebugLineOut(opcode.RAW, "disable extended mode (SUPER)");
	ProfilerUpdate(41);

	Mode_SChip = false;
	UpdateRect();
}

void Machine::_S00FF() // Enable extended screen mode for full-screen graphics
{
	DebugLineOut(opcode.RAW, "enable extended mode (SUPER)");
	ProfilerUpdate(42);

	Mode_SChip = true;
	UpdateRect();
}

// DXY0 ... // If N=0 and extended mode, show 16x16 sprite.

void Machine::_SFX30() // Point I to 10-byte font sprite for digit VX (0..9)
{
	DebugLineOut(opcode.RAW, "point I to 10-byte digit (SUPER)", opcode.X);
	ProfilerUpdate(43);

	I =  80 + V[opcode.X] * 10;
}

void Machine::_SFX75() // Store V0..VX in RPL user flags (X <= 7)
{
	DebugLineOut(opcode.RAW, "store V0 to V%X in HP48 (SUPER)", opcode.X);
	ProfilerUpdate(44);

	for(u8 i = 0; i <= opcode.X && i < 8; i++)
		HP48[i] = V[i];
}

void Machine::_SFX85() // Read V0..VX from RPL user flags (X <= 7) 
{
	DebugLineOut(opcode.RAW, "read V0 to V%X from HP48 (SUPER)", opcode.X);
	ProfilerUpdate(45);

	for(u8 i = 0; i <= opcode.X && i < 8; i++)
		V[i] = HP48[i];
}

/*
 * MEGA CHIP!

 
MEGAChip Specs:
- 256x192 resolution
- Indexed coloring (255 colors max + transparency)
- Fixed high-speed speed in megachip mode
- Custom sprite sizes
- Update timing at ClearScreen
- Extended I-register range (24 bit addessing, 32MB max)
- Digitised sound (mono 8bit)
- Downward compability (you can run your old CHIP/S-CHIP games)
- Spritecolor 0 = transparent.
- Spritecollision will occur if (backgroundcolor>0) when plotting spritepixel.
 */

void Machine::_M0010() // Disable Megachip mode (MEGAoFF)
{
	DebugLineOut(opcode.RAW, "disable MegaChip mode (MEGA)");
	ProfilerUpdate(46);

	Mode_Mega = false;
	UpdateRect();
}

void Machine::_M0011() // Enable Megachip mode (MEGAON)
{
	DebugLineOut(opcode.RAW, "enable MegaChip mode (MEGA)");
	ProfilerUpdate(47);

	Mode_Mega = true;
	UpdateRect();
}

void Machine::_M01NN() // I=(nn<<16)+nnnn , PC+=2; (LDHI I,nnnnnn , always follow LDHI with a NOP)
{
	if(!Mode_Mega) { _0NNN(); return; }

	DebugLineOut(opcode.RAW, "LDHI (MEGA)");
	ProfilerUpdate(48);

	I = opcode.NN << 16;
	
	GetOpcode();

	I |= opcode.RAW;

	I = Wrap<int>(I, 0x200, 0xFFFFFF);
}

void Machine::_M02NN() // Load nn-colors palette at I (LDPAL nn)
{
	if(!Mode_Mega) { _0NNN(); return; }

	DebugLineOut(opcode.RAW, "LDPAL %2X (MEGA)", opcode.NN);
	ProfilerUpdate(49);

	u8 *mem =  &Memory[I];

	for(u16 i = 0; i <= opcode.NN; i++)
	{
		Palette[i]  = *mem << 24;	mem++; // A
		Palette[i] |= *mem << 16;	mem++; // R
		Palette[i] |= *mem << 8;	mem++; // G
		Palette[i] |= *mem;			mem++; // B
	}

	LoadPalette();
}

void Machine::_M03NN() // Set Sprite-width to nn (SPRW nn)
{
	if(!Mode_Mega) { _0NNN(); return; }

	DebugLineOut(opcode.RAW, "set spriteW %d (MEGA)", opcode.NN);
	ProfilerUpdate(50);

	SpriteW = opcode.NN == 0? 256 : opcode.NN;
}

void Machine::_M04NN() // Set Sprite-height to nn (SPRH nn)
{
	if(!Mode_Mega) { _0NNN(); return; }

	DebugLineOut(opcode.RAW, "set spriteH %d (MEGA)", opcode.NN);
	ProfilerUpdate(51);

	SpriteH = opcode.NN;
}

void Machine::_M05NN() // Set Screenalpha to nn (ALPHA nn, will become FADE nn)
{
	if(!Mode_Mega) { _0NNN(); return; }

	DebugLineOut(opcode.RAW, "set screenAlpha %d (MEGA)", opcode.NN);
	ProfilerUpdate(52);
}

void Machine::_M060N() // Play digitised sound at I (DIGISND), will add n for loop/noloop
{
	if(!Mode_Mega) { _0NNN(); return; }

	DebugLineOut(opcode.RAW, "DIGISND (MEGA)");
	ProfilerUpdate(53);

	if(opcode.Y)
		_0NNN();
	else
	{
	}
}

void Machine::_M0700() // Stop digitised sound (STOPSND)
{
	if(!Mode_Mega) _0NNN();

	DebugLineOut(opcode.RAW, "STOPSND (MEGA)");
	ProfilerUpdate(54);

	if(opcode.NN)
		_0NNN();
	else
	{
	}
}

void Machine::_M080N() // Set sprite blendmode (BMODE n) (0=normal,1=25%,2=50%,3=75%,4=addative,5=multiply)
{
	if(!Mode_Mega) _0NNN();

	DebugLineOut(opcode.RAW, "Set sprite blend %d", opcode.N);
	ProfilerUpdate(55);

	if(opcode.Y)
		_0NNN();
	else
	{
	}
}

void Machine::_M00BN() // Scroll display N lines up (SCRU n)
{
	if(!Mode_Mega) _0NNN();

	DebugLineOut(opcode.RAW, "scroll up %d (MEGA)", opcode.N);
	ProfilerUpdate(56);
}

void Machine::_MDXYN()
{
	DebugLineOut(opcode.RAW, "draw\t(%d, %d, %d) (%d, %d) (MEGA)", opcode.X, opcode.Y, opcode.N, SpriteW, SpriteH);
	ProfilerUpdate(57);

	const u8 ox = V[opcode.X];
	const u8 oy = V[opcode.Y];

	V[0xF] = 0;

	u8 *mem = &Memory[I];
	u16 nx, ny;

	for(u16 y = 0; y < SpriteH; y++)
	{
		for(u16 x = 0; x < SpriteW; x++)
		{
			nx = (ox + x) % 256;
			ny = (oy + y) % 192;

			Display[nx][ny] = *mem; mem++;
			SetPixel(nx, ny, --Display[nx][ny]);
		}
	}

	//RenderSprite();
}

/*
 * For the function array
 *
 */

void Machine::_NULL()
{
	ProfilerUpdate(58);
}

void Machine::_00XX()
{
	switch(opcode.Y)
	{
	case 0x1: switch(opcode.N)
		{
		case 0x0: _M0010(); return;
		case 0x1: _M0011(); return;
		default: _0NNN(); return;
		};

	case 0xB: _M00BN(); return;
	case 0xC: _S00CN(); return;
	case 0xE: switch(opcode.N)
		{
		case 0x0: _00E0(); return;
		case 0xE: _00EE(); return;
		default: _0NNN(); return;
		};

	case 0xF: switch(opcode.N)
		{
		case 0xB: _S00FB(); return;
		case 0xC: _S00FC(); return;
		case 0xD: _S00FD(); return;
		case 0xE: _S00FE(); return;
		case 0xF: _S00FF(); return;
		default: _0NNN(); return;
		};

	default: _0NNN(); return;
	}
}

void Machine::_0XXX()
{
	if((opcode.RAW & 0xFFF) == 0x0230)
		_H0230();
	else 
		(this->*I0X00[opcode.X])();
}

void Machine::_1XXX()
{
	if(PC == 0x202 && opcode.RAW == 0x1260)
		_H1260(); 
	else
		_1NNN(); 
}

void Machine::_8XXX()
{
	(this->*I800X[opcode.N])();
}

void Machine::_EXXX()
{
	switch(opcode.NN)
	{
	case 0x9E: _EX9E(); return;
	case 0xA1: _EXA1(); return;
	}
}

void Machine::_FXXX()
{
	(this->*IF0XX[opcode.NN])();
}

void Machine::_DRAW()
{
	if(Mode_Mega)
		_MDXYN();
	else
		_DXYN();
}