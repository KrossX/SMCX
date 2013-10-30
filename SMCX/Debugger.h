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

void _DebuggerStart();
void _DebuggerStop();
void _DebugLineIn(u8 *V, u8 *HP, u16 PC, u32 I);
void _DebugLineOut(u16 opcode, char *fmt, ...);

void _ProfilerStart();
void _ProfilerStop();
void _ProfilerUpdate(u16 fnID);

//#define DEBUGGER
//#define PROFILER

#ifdef DEBUGGER
	#define DebuggerStart _DebuggerStart
	#define DebuggerStop _DebuggerStop
	#define DebugLineIn _DebugLineIn
	#define DebugLineOut _DebugLineOut
#else
	#define DebuggerStart(...) 
	#define DebuggerStop(...)
	#define DebugLineIn(...)
	#define DebugLineOut(...)
#endif

#ifdef PROFILER
	#define ProfilerStart _ProfilerStart
	#define ProfilerStop _ProfilerStop
	#define ProfilerUpdate _ProfilerUpdate
#else
	#define ProfilerStart()
	#define ProfilerStop()
	#define ProfilerUpdate(x)
#endif

