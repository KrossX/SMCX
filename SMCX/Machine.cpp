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
#include <ctime>
#include "Renderer.h"

MACHINE::MACHINE()
{
	IX000[0x0] = &MACHINE::_0XXX;
	IX000[0x1] = &MACHINE::_1XXX;
	IX000[0x2] = &MACHINE::_2NNN;
	IX000[0x3] = &MACHINE::_3XNN;
	IX000[0x4] = &MACHINE::_4XNN;
	IX000[0x5] = &MACHINE::_5XY0;
	IX000[0x6] = &MACHINE::_6XNN;
	IX000[0x7] = &MACHINE::_7XNN;
	IX000[0x8] = &MACHINE::_8XXX;
	IX000[0x9] = &MACHINE::_9XY0;
	IX000[0xA] = &MACHINE::_ANNN;
	IX000[0xB] = &MACHINE::_BNNN;
	IX000[0xC] = &MACHINE::_CXNN;
	IX000[0xD] = &MACHINE::_DRAW;
	IX000[0xE] = &MACHINE::_EXXX;
	IX000[0xF] = &MACHINE::_FXXX;

	for (int i = 0; i < 16; i++)
	{
		I0X00[i] = &MACHINE::_0NNN;
		I800X[i] = &MACHINE::_NULL;
	}

	for (int i = 0; i < 256; i++)
	{
		IF0XX[i] = &MACHINE::_NULL;
	}

	I0X00[0x0] = &MACHINE::_00XX;
	I0X00[0x1] = &MACHINE::_M01NN;
	I0X00[0x2] = &MACHINE::_M02NN;
	I0X00[0x3] = &MACHINE::_M03NN;
	I0X00[0x4] = &MACHINE::_M04NN;
	I0X00[0x5] = &MACHINE::_M05NN;
	I0X00[0x6] = &MACHINE::_M060N;
	I0X00[0x7] = &MACHINE::_M0700;
	I0X00[0x8] = &MACHINE::_M080N;

	I800X[0x0] = &MACHINE::_8XY0;
	I800X[0x1] = &MACHINE::_8XY1;
	I800X[0x2] = &MACHINE::_8XY2;
	I800X[0x3] = &MACHINE::_8XY3;
	I800X[0x4] = &MACHINE::_8XY4;
	I800X[0x5] = &MACHINE::_8XY5;
	I800X[0x6] = &MACHINE::_8XY6;
	I800X[0x7] = &MACHINE::_8XY7;
	I800X[0xE] = &MACHINE::_8XYE;

	IF0XX[0x07] = &MACHINE::_FX07;
	IF0XX[0x0A] = &MACHINE::_FX0A;
	IF0XX[0x15] = &MACHINE::_FX15;
	IF0XX[0x18] = &MACHINE::_FX18;
	IF0XX[0x1E] = &MACHINE::_FX1E;
	IF0XX[0x29] = &MACHINE::_FX29;
	IF0XX[0x30] = &MACHINE::_SFX30;
	IF0XX[0x33] = &MACHINE::_FX33;
	IF0XX[0x55] = &MACHINE::_FX55;
	IF0XX[0x65] = &MACHINE::_FX65;
	IF0XX[0x75] = &MACHINE::_SFX75;
	IF0XX[0x85] = &MACHINE::_SFX85;
}

u8 MACHINE::GetMode()
{
	if (Mode_Mega)
		return CMODE_MEGA;
	else if (Mode_SChip)
		return CMODE_SCHIP;
	else if (Mode_Chip8H)
		return CMODE_CHIP8H;
	else
		return CMODE_CHIP8;
}

void MACHINE::UpdateRect()
{
	switch (GetMode())
	{
	case CMODE_CHIP8:
		ScaleX = 4;
		ScaleY = 6;
		break;

	case CMODE_CHIP8H:
		ScaleX = 4;
		ScaleY = 3;
		break;

	case CMODE_SCHIP:
		ScaleX = 2;
		ScaleY = 3;
		break;

	case CMODE_MEGA:
		ScaleX = 1;
		ScaleY = 1;
		break;
	}
}

void MACHINE::Reset()
{
	memset(Display, 0, sizeof(Display));
	memset(V, 0, sizeof(V));
	memset(HP48, 0, sizeof(HP48));
	memset(Input, 0, sizeof(Input));
	memset(Stack, 0, sizeof(Stack));

	PC = 0x200;
	I = 0x200;

	DelayTimer = 0;
	SoundTimer = 0;

	SP = 0;

	Mode_Mega = false;
	Mode_SChip = false;
	Mode_Chip8H = false;
	UpdateRect();

	srand((int)time(NULL));
	RenderClear();
	RenderSprite();
}



void MACHINE::Loop()
{
	DebugLineIn(V, HP48, PC, I);

	QueryPerformanceCounter(&tNew);
	tDiff = tNew.QuadPart - tOld.QuadPart;

	bool msmooth = Mode_Mega && Mega_Smooth;

	if (msmooth || !tFreq || tDiff > tFreq)
	{
		GetOpcode();
		(this->*IX000[opcode.C])();
		tOld = tNew;
	}

	tDiff = tNew.QuadPart - rOld.QuadPart;

	if (!msmooth && (!rFreq || tDiff > rFreq))
	{
		RenderSprite();
		rOld = tNew;
	}
}

struct _timers
{
	u8 *DT, *ST;
} timers;

extern RENDERER * render;

void CountdownThread()
{
	const u8 wait = 1000 / 60;
	const bool loop = timers.DT && timers.ST;

	while (loop)
	{
		if (*timers.DT) *timers.DT -= 1;
		if (*timers.ST) *timers.ST -= 1;

		Sleep(wait);
	};
}

void MACHINE::Start()
{
	DebuggerStart();
	ProfilerStart();

	timers.DT = &DelayTimer;
	timers.ST = &SoundTimer;

	hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CountdownThread, 0, 0, NULL);

	QueryPerformanceCounter(&tOld);
	QueryPerformanceFrequency(&_Freq);

	rOld = tOld;

	SetCPUfreq(600);
	SetGPUfreq(60);

	Mega_Smooth = true;
	vsync = 0;

	Reset();
}

void MACHINE::Shutdown()
{
	DebuggerStop();
	ProfilerStop();

	if (hThread) TerminateThread(hThread, 0);
}

void MACHINE::SetCPUfreq(u32 freq)
{
	cpuFreq = freq;
	tFreq = cpuFreq ? _Freq.QuadPart / cpuFreq : 0;
}

void MACHINE::SetGPUfreq(u32 freq)
{
	gpuFreq = freq;
	rFreq = gpuFreq ? _Freq.QuadPart / gpuFreq : 0;
}

