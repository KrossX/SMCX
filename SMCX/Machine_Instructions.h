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

/*
 * CHIP8
 */

void _0NNN();
void _00E0();
void _00EE();
void _1NNN();
void _2NNN();
void _3XNN();
void _4XNN();
void _5XY0();
void _6XNN();
void _7XNN();
void _8XY0();
void _8XY1();
void _8XY2();
void _8XY3();
void _8XY4();
void _8XY5();
void _8XY6();
void _8XY7();
void _8XYE();
void _9XY0();
void _ANNN();
void _BNNN();
void _CXNN();
void _DXYN();
void _EX9E();
void _EXA1();
void _FX07();
void _FX0A();
void _FX15();
void _FX18();
void _FX1E();
void _FX29();
void _FX33();
void _FX55();
void _FX65();

/*
 * CHIP8 - Hi Res
 */

void _H1260();
void _H0230();

/*
 * SUPER-CHIP8
 */

void _S00CN();
void _S00FB();
void _S00FC();
void _S00FD();
void _S00FE();
void _S00FF();
void _SFX30();
void _SFX75();
void _SFX85();

/*
 * MEGA CHIP!
 */

void _M0010();
void _M0011();
void _M01NN();
void _M02NN();
void _M03NN();
void _M04NN();
void _M05NN();
void _M060N();
void _M0700();
void _M080N();
void _M00BN();
void _MDXYN();

/*
 * For the function array
 *
 */

void _NULL();
void _00XX();
void _0XXX();
void _1XXX();
void _8XXX();
void _EXXX();
void _FXXX();
void _DRAW();