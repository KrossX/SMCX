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
#include "debugger.h"

#include <cstdio>
FILE * dbFile = NULL;

u64 profilerData[64];
DWORD initTicks = 0;


void _DebuggerStart()
{
	dbFile = fopen("log.log", "w");
}

void _DebuggerStop()
{
	if(dbFile) fclose(dbFile);
}

void _DebugLineIn(u8 *V, u8 *HP, u16 PC, u32 I)
{
	if(dbFile)
	{
		char line[2048] = {0};
		sprintf_s(line, "V[%2X|%2X|%2X|%2X|%2X|%2X|%2X|%2X|%2X|%2X||%2X|%2X|%2X|%2X|%2X||%2X] [%3X|%3X]\t", 
			V[0], V[1], V[2],  V[3],  V[4],  V[5],  V[6],  V[7],  V[8],  V[9],  V[10],  V[11],  V[12],  V[13],  V[14],  V[15],
			PC, I);
		fputs(line, dbFile);
		fflush(dbFile);
	}
}

void _DebugLineOut(u16 opcode, char *fmt, ...)
{
	if(dbFile)
	{
		char line0[2048] = {0};
		char line1[2048] = {0};

		va_list args;
		va_start(args, fmt);
		vsprintf_s(line0, fmt, args);
		va_end(args);

		sprintf_s(line1, "%04X %s\n", opcode, line0);
		
		fputs(line1, dbFile);
		fflush(dbFile);
	}
}

void _ProfilerStart()
{
	memset(profilerData, 00, sizeof(profilerData));
	initTicks = GetTickCount();
}

void _ProfilerStop()
{
	FILE * prFile = fopen("profiler.log", "w");

	const DWORD newTicks = GetTickCount();
	const DWORD diffTicks = newTicks - initTicks;

	u64 total = 0;

	if(prFile)
	{
		char line[2048];

		for(int i = 0; i < 64; i++)
			total += profilerData[i];

		for(int i = 0; i < 64; i++)
		{
			sprintf_s(line, "%2d:", i+1);
			sprintf_s(line, "%s %10ld,", line, profilerData[i]);
			sprintf_s(line, "%s %8.5f,", line, profilerData[i] / (double)total * 100.0);
			sprintf_s(line, "%s %12.5f\n", line, profilerData[i] / (double)diffTicks);
			
			fputs(line, prFile);
		}

		sprintf_s(line, "Total: %d ||", total);
		sprintf_s(line, "%s Time: %d\n", line, diffTicks);
		fputs(line, prFile);

		fclose(prFile);
	}
}

void _ProfilerUpdate(u16 fnID)
{
	profilerData[fnID]++;
}