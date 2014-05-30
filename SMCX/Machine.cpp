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


#include "main.h"
#include "machine.h"
#include <ctime>
#include "renderer.h"

Machine::Machine()
{
	IX000[0x0] = &Machine::_0XXX;
	IX000[0x1] = &Machine::_1XXX;
	IX000[0x2] = &Machine::_2NNN;
	IX000[0x3] = &Machine::_3XNN;
	IX000[0x4] = &Machine::_4XNN;
	IX000[0x5] = &Machine::_5XY0;
	IX000[0x6] = &Machine::_6XNN;
	IX000[0x7] = &Machine::_7XNN;
	IX000[0x8] = &Machine::_8XXX;
	IX000[0x9] = &Machine::_9XY0;
	IX000[0xA] = &Machine::_ANNN;
	IX000[0xB] = &Machine::_BNNN;
	IX000[0xC] = &Machine::_CXNN;
	IX000[0xD] = &Machine::_DRAW;
	IX000[0xE] = &Machine::_EXXX;
	IX000[0xF] = &Machine::_FXXX;

	for (int i = 0; i < 16; i++)
	{
		I0X00[i] = &Machine::_0NNN;
		I800X[i] = &Machine::_NULL;
	}

	for (int i = 0; i < 256; i++)
	{
		IF0XX[i] = &Machine::_NULL;
	}

	I0X00[0x0] = &Machine::_00XX;
	I0X00[0x1] = &Machine::_M01NN;
	I0X00[0x2] = &Machine::_M02NN;
	I0X00[0x3] = &Machine::_M03NN;
	I0X00[0x4] = &Machine::_M04NN;
	I0X00[0x5] = &Machine::_M05NN;
	I0X00[0x6] = &Machine::_M060N;
	I0X00[0x7] = &Machine::_M0700;
	I0X00[0x8] = &Machine::_M080N;

	I800X[0x0] = &Machine::_8XY0;
	I800X[0x1] = &Machine::_8XY1;
	I800X[0x2] = &Machine::_8XY2;
	I800X[0x3] = &Machine::_8XY3;
	I800X[0x4] = &Machine::_8XY4;
	I800X[0x5] = &Machine::_8XY5;
	I800X[0x6] = &Machine::_8XY6;
	I800X[0x7] = &Machine::_8XY7;
	I800X[0xE] = &Machine::_8XYE;

	IF0XX[0x07] = &Machine::_FX07;
	IF0XX[0x0A] = &Machine::_FX0A;
	IF0XX[0x15] = &Machine::_FX15;
	IF0XX[0x18] = &Machine::_FX18;
	IF0XX[0x1E] = &Machine::_FX1E;
	IF0XX[0x29] = &Machine::_FX29;
	IF0XX[0x30] = &Machine::_SFX30;
	IF0XX[0x33] = &Machine::_FX33;
	IF0XX[0x55] = &Machine::_FX55;
	IF0XX[0x65] = &Machine::_FX65;
	IF0XX[0x75] = &Machine::_SFX75;
	IF0XX[0x85] = &Machine::_SFX85;
}

u8 Machine::get_mode()
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

void Machine::update_rect()
{
	switch (get_mode())
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

void Machine::reset()
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
	update_rect();

	srand((int)time(NULL));
	render_clear();
	render_sprite();
}



void Machine::loop()
{
	DebugLineIn(V, HP48, PC, I);

	QueryPerformanceCounter(&tNew);
	tDiff = tNew.QuadPart - tOld.QuadPart;

	bool msmooth = Mode_Mega && mega_smooth;

	if (msmooth || !tFreq || tDiff > tFreq)
	{
		get_opcode();
		(this->*IX000[opcode.C])();
		tOld = tNew;
	}

	tDiff = tNew.QuadPart - rOld.QuadPart;

	if (!msmooth && (!rFreq || tDiff > rFreq))
	{
		render_sprite();
		rOld = tNew;
	}
}

struct _timers
{
	u8 *DT, *ST;
} timers;

extern Renderer * render;

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

void Machine::start()
{
	DebuggerStart();
	ProfilerStart();

	timers.DT = &DelayTimer;
	timers.ST = &SoundTimer;

	hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CountdownThread, 0, 0, NULL);

	QueryPerformanceCounter(&tOld);
	QueryPerformanceFrequency(&_Freq);

	rOld = tOld;

	set_cpu_freq(600);
	set_gpu_freq(60);

	mega_smooth = true;
	vsync = 0;

	reset();
}

void Machine::shutdown()
{
	DebuggerStop();
	ProfilerStop();

	if (hThread) TerminateThread(hThread, 0);
}

void Machine::set_cpu_freq(u32 freq)
{
	cpu_freq = freq;
	tFreq = cpu_freq ? _Freq.QuadPart / cpu_freq : 0;
}

void Machine::set_gpu_freq(u32 freq)
{
	gpu_freq = freq;
	rFreq = gpu_freq ? _Freq.QuadPart / gpu_freq : 0;
}

