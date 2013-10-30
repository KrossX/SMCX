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


#pragma once
#include "Debugger.h"

union OP // Opcode
{
	struct
	{
		u16 N : 4;
		u16 Y : 4;
		u16 X : 4;
		u16 C : 4;
	};

	struct
	{
		u16 NN : 8;
		u16 CC : 8;
	};

	u16 RAW;
};

enum
{
	CMODE_CHIP8,
	CMODE_CHIP8H,
	CMODE_SCHIP,
	CMODE_MEGA
};

template<class T>
T Clamp(T value, T min, T max)
{
	return value > max? max : value < min? min : value;
};

template<class T>
T Wrap(T value, T min, T max)
{
	if(value > max) Wrap(value - max +min, min, max);
	if(value < min) Wrap(value + max -min, min, max);

	return value;
};


class Machine
{
	u8 Memory[0x2000000 /*0x1000*/]; // 4 KB of Memory, MEGA 32 MB?
	u8 Display[256][192]; // Black and White display Chip 8 (64x32) SCHIP (128x64)
						 // Mega (256x192), indexed colors and transparency
	u32 Palette[256]; // MEGA Colors!

	u8 HP48[8]; // Super Flags!
	u8 V[16]; // Registers
	u8 Input[16]; // 16 keys... 0-9, A-F
	u16 Stack[16]; // Stack, 16 levels of nesting

	typedef void (Machine::* pFunc)();
	pFunc I0X00[16];
	pFunc IX000[16];
	pFunc I800X[16];

	pFunc IF0XX[256];

	u32 I; // Memory Addressing Register
			// MEGA: 24bit address, 32MB MAX
	
	u16 SpriteW, SpriteH; // MEGA

	u16 PC; // Program Counter
	
	u8 DelayTimer, SoundTimer;
	u8 SP; // Stack Pointer

	// For drawing...
	u8 ScaleX, ScaleY; 

	bool Mode_Chip8H;
	bool Mode_SChip;
	bool Mode_Mega;

	OP opcode;
	HANDLE hThread;

	// Timer stuff...
	LARGE_INTEGER tOld, tNew, rOld, _Freq;
	LONGLONG tDiff, tFreq, rFreq;
	



#include "Machine_Instructions.h"

	void GetOpcode() 
	{ 
		opcode.CC = Memory[PC++];
		opcode.NN = Memory[PC++];
	}

	u8 GetMode();
	void UpdateRect();

	u16 GetRandom();

	void SetPixel(u16 x, u16 y, u8 pix);
	void RenderClear();
	void RenderSprite();
	void LoadPalette();

public:
	
	// Dialog stuff...
	bool Mega_Smooth;
	u8 vsync; 
	u32 cpuFreq, gpuFreq;

	void SetCPUfreq(u32 freq); // ops per second
	void SetGPUfreq(u32 freq); // frames per second
	
	void UpdateInput(u8 key, u8 state);
	bool LoadROM(const wchar_t * name);
	void Start();
	void Reset();
	void Loop();
	void Shutdown();

	Machine();
};