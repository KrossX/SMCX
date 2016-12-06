/* Copyright (c) 2016 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "chip16.h"
#include "sound.h"

#include <stdint.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

#define MEMSIZE  (64 * 1024)
#define WIDTH    (320)
#define HEIGHT   (240)
#define DISPSIZE (WIDTH * HEIGHT)

static uint8_t   MEM[0x10000];                     // 64KB RAM
static uint16_t *STACK  = (uint16_t*)&MEM[0xFDF0]; // 512B stack
static uint32_t *IOPORT = (uint32_t*)&MEM[0xFFF0]; // 4B MMAP IO ports x4

static uint8_t  DISPLAY[DISPSIZE]; // FG display layer, 8-bit indexed
static uint32_t DISPCOL[DISPSIZE]; // Color output
static uint32_t PALETTE[16];       // Color palette
static uint8_t  BG;                // BG color index
static uint8_t  SPRW, SPRH;        // Sprite width, height
static uint8_t  FLIPH, FLIPV;      // Flip horizontal, vertical

static uint16_t PC;
static uint8_t  SP;
static uint8_t  VBLNK;

// FLAGS
// 0 Carry
// 1 Zero
// 6 Overflow
// 7 Negative

static uint8_t FC, FZ, FO, FN;
static uint16_t R[16];

int32_t TEMP;

void  chip16_input(int key, int state)
{
	uint8_t PAD1 = IOPORT[0] & 0xFF;

	switch(key)
	{
	case 0x26: PAD1 = state? PAD1 | (1<<0) : PAD1 & ~(1<<0); break; // UP
	case 0x28: PAD1 = state? PAD1 | (1<<1) : PAD1 & ~(1<<1); break; // DOWN
	case 0x25: PAD1 = state? PAD1 | (1<<2) : PAD1 & ~(1<<2); break; // LEFT
	case 0x27: PAD1 = state? PAD1 | (1<<3) : PAD1 & ~(1<<3); break; // RIGHT
	
	case 'C': PAD1 = state? PAD1 | (1<<4) : PAD1 & ~(1<<4); break; // SELECT
	case 'V': PAD1 = state? PAD1 | (1<<5) : PAD1 & ~(1<<5); break; // START
	case 'Z': PAD1 = state? PAD1 | (1<<6) : PAD1 & ~(1<<6); break; // A
	case 'X': PAD1 = state? PAD1 | (1<<7) : PAD1 & ~(1<<7); break; // B
	}

	IOPORT[0] = (IOPORT[0] & 0xFFFFFF00) | PAD1;
}

void chip16_update_display(void)
{
	int i;
	uint8_t index;

	for(i = 0; i < DISPSIZE; i++)
	{
		index = DISPLAY[i] & 0xF;
		if(!index) index = BG;
		DISPCOL[i] = PALETTE[index];
	}
}

void chip16_load_palette(uint8_t *m)
{
	int i;

	for(i = 0; i < 16; i++) // RGB to RGBA
	{
		PALETTE[i] = (0xFF << 24) | (m[2] << 16) | (m[1] << 8) | m[0];
		m += 3;
	}
	
}

void chip16_draw(int X, int Y, uint8_t *m)
{
	int y, y0 = Y, y1 = Y + SPRH, yinc = 1;
	int x, x0 = X, x1 = X + SPRW * 2, xinc = 1;
	uint8_t pix;

	if(FLIPH) { xinc = x0; x0 = x1; x1 = xinc; xinc = -1; }
	if(FLIPV) { yinc = y0; y0 = y1; y1 = yinc; yinc = -1; }

	FC = 0;

	for(y = y0; y != y1; y+=yinc)
	{
		uint8_t *d = &DISPLAY[y * 320];

		if(y >= 0 && y < 240)
			for(x = x0; x != x1; x+=xinc)
			{
				if(x >= 0 && x < 320)
				{
					pix = (*m >> 4) & 0xF;
					if(pix) { if(d[x]) FC = 1; d[x] = pix; }
				}

				x+=xinc;

				if(x >= 0 && x < 320)
				{
					pix = *m & 0xF;
					if(pix) { if(d[x]) FC = 1; d[x] = pix; }
				}

				m++;
			}
		else
			m += SPRW;
	}
}

void chip16_call(uint16_t HHLL)
{
	STACK[SP++] = PC;
	PC = HHLL; 
}

void chip16_return(void)
{
	PC = STACK[--SP];
}

void chip16_flag(uint8_t flags)
{
	if(flags & 0x01) // Carry
		FC = (TEMP >> 16) & 1;

	if(flags & 0x02) // Zero
		FZ = (TEMP & 0xFFFF) ? 0 : 1;

	if(flags & 0x40) // Overflow
		FO = ((TEMP >> 14) ^ (TEMP >> 15)) & 1;

	if(flags & 0x80) // Negative
		FN = (TEMP >> 15) & 1;
}

uint16_t chip16_getflags(void)
{
	return (FN << 7) | (FO << 6) | (FZ << 1) | FC;
}

void chip16_setflags(uint16_t in)
{
	FC = in & 1;
	FZ = (in >> 1) & 1;
	FO = (in >> 6) & 1;
	FN = (in >> 7) & 1;
}

int chip16_branch(int index)
{
	switch(index)
	{
	case 0x0: return       FZ;
	case 0x1: return 1 -   FZ;
	case 0x2: return       FN;
	case 0x3: return 1 -   FN;
	case 0x4: return 1 -  (FZ | FN);
	case 0x5: return       FO;
	case 0x6: return 1 -   FO;
	case 0x7: return 1 -  (FC | FZ);
	case 0x8: return 1 -   FC;
	case 0x9: return       FC;
	case 0xA: return      (FC | FZ);
	case 0xB: return 1 - ((FO ^ FN) | FZ);
	case 0xC: return 1 -  (FO ^ FN);
	case 0xD: return      (FO ^ FN);
	case 0xE: return     ((FO ^ FN) | FZ);
	//case 0xF: return 0;
	}

	return 0;
}

uint16_t ADD(uint16_t X, uint16_t Y)
{
	TEMP = X + Y; chip16_flag(0xC3);
	return TEMP & 0xFFFF;
}

uint16_t SUB(uint16_t X, uint16_t Y)
{
	TEMP = X - Y; chip16_flag(0xC3);
	return TEMP & 0xFFFF;
}

uint16_t AND(uint16_t X, uint16_t Y)
{
	TEMP = X & Y; chip16_flag(0x82);
	return TEMP & 0xFFFF;
}

uint16_t  OR(uint16_t X, uint16_t Y)
{
	TEMP = X | Y; chip16_flag(0x82);
	return TEMP & 0xFFFF;
}

uint16_t XOR(uint16_t X, uint16_t Y)
{
	TEMP = X ^ Y; chip16_flag(0x82);
	return TEMP & 0xFFFF;
}

uint16_t MUL(uint16_t X, uint16_t Y)
{
	TEMP = X * Y; chip16_flag(0x83);
	return TEMP & 0xFFFF;
}

uint16_t DIV(uint16_t X, uint16_t Y)
{
	double t = X / (double)Y;
	TEMP = (int32_t)t; chip16_flag(0x82);
	FC = (t - TEMP) ? 1 : 0;
	return TEMP & 0xFFFF;
}

uint16_t MOD(int16_t X, int16_t Y) // WTF?
{
	TEMP = X % Y + (((X^Y) >> 15)&1) * Y; chip16_flag(0x82);
	return TEMP & 0xFFFF;
}

uint16_t REM(int16_t X, int16_t Y)
{
	TEMP = X % Y; chip16_flag(0x82);
	return TEMP & 0xFFFF;
}

uint16_t SHL(uint16_t X, uint16_t Y)
{
	TEMP = X << Y; chip16_flag(0x82);
	return TEMP & 0xFFFF;
}

uint16_t SHR(uint16_t X, uint16_t Y)
{
	TEMP = X >> Y; chip16_flag(0x82);
	return TEMP & 0xFFFF;
}

uint16_t SAR(int16_t X, uint16_t Y)
{
	TEMP = X >> Y; chip16_flag(0x82);
	return TEMP & 0xFFFF;
}

uint16_t NOT(uint16_t X)
{
	TEMP = ~X; chip16_flag(0x82);
	return TEMP & 0xFFFF;
}

uint16_t NEG(uint16_t X)
{
	TEMP = -X; chip16_flag(0x82);
	return TEMP & 0xFFFF;
}


void chip16_decode(uint32_t op)
{
	uint8_t OP =  op & 0xFF;
	uint8_t  X = (op >>  8) & 0xF;
	uint8_t  Y = (op >> 12) & 0xF;
	uint8_t  Z = (op >> 16) & 0xF;
	uint8_t LL = (op >> 16) & 0xFF;
	uint8_t HH = (op >> 24) & 0xFF;
	uint16_t HHLL = HH << 8 | LL;

	switch(OP)
	{
	case 0x00: break; // NOP
	case 0x01: memset(DISPLAY, 0, DISPSIZE); BG = 0; break;
	case 0x02: PC -= (1 - VBLNK) * 4; break;
	case 0x03: BG = LL & 0xF; break;
	case 0x04: SPRW = LL; SPRH = HH; break;
	case 0x05: chip16_draw((signed)R[X], (signed)R[Y], &MEM[HHLL]); break;
	case 0x06: chip16_draw((signed)R[X], (signed)R[Y], &MEM[R[Z]]);	break;
	case 0x07: R[X] = rand() % (HHLL+1); break;
	case 0x08: FLIPH = (HHLL>>9)&1; FLIPV = (HHLL>>8)&1; break;
	case 0x09: snd_stop(); break;
	case 0x0A: snd_beep( 500, HHLL); break;
	case 0x0B: snd_beep(1000, HHLL); break;
	case 0x0C: snd_beep(1500, HHLL); break;
	case 0x0D: snd_beep(*(uint16_t*)&MEM[R[X]], HHLL); break;
	case 0x0E: snd_adrt(Y, X, Z, HH & 0xFF); snd_vol(HH >> 4, LL >> 4); break;

	case 0x10: PC = HHLL; break;
	case 0x11: if(FC) PC = HHLL; break;
	case 0x12: if(chip16_branch(X)) PC = HHLL; break;
	case 0x13: if(R[X] == R[Y]) PC = HHLL; break;
	case 0x14: chip16_call(HHLL); break;
	case 0x15: chip16_return(); break;
	case 0x16: PC = R[X]; break;
	case 0x17: if(chip16_branch(X)) chip16_call(HHLL); break;
	case 0x18: chip16_call(R[X]); break;

	case 0x20: R[X] = HHLL; break;
	case 0x21: SP = (HHLL >> 1) & 0xFF; break;
	case 0x22: R[X] = *(uint16_t*)&MEM[HHLL]; break;
	case 0x23: R[X] = *(uint16_t*)&MEM[R[Y]]; break;
	case 0x24: R[X] = R[Y]; break;

	case 0x30: *(uint16_t*)&MEM[HHLL] = R[X]; break;
	case 0x31: *(uint16_t*)&MEM[R[Y]] = R[X]; break;

	case 0x40: R[X] = ADD(R[X], HHLL); break;
	case 0x41: R[X] = ADD(R[X], R[Y]); break;
	case 0x42: R[Z] = ADD(R[X], R[Y]); break;

	case 0x50: R[X] = SUB(R[X], HHLL); break;
	case 0x51: R[X] = SUB(R[X], R[Y]); break;
	case 0x52: R[Z] = SUB(R[X], R[Y]); break;
	case 0x53:        SUB(R[X], HHLL); break;
	case 0x54:        SUB(R[X], R[Y]); break;

	case 0x60: R[X] = AND(R[X], HHLL); break;
	case 0x61: R[X] = AND(R[X], R[Y]); break;
	case 0x62: R[Z] = AND(R[X], R[Y]); break;
	case 0x63:        AND(R[X], HHLL); break;
	case 0x64:        AND(R[X], R[Y]); break;

	case 0x70: R[X] =  OR(R[X], HHLL); break;
	case 0x71: R[X] =  OR(R[X], R[Y]); break;
	case 0x72: R[Z] =  OR(R[X], R[Y]); break;

	case 0x80: R[X] = XOR(R[X], HHLL); break;
	case 0x81: R[X] = XOR(R[X], R[Y]); break;
	case 0x82: R[Z] = XOR(R[X], R[Y]); break;

	case 0x90: R[X] = MUL(R[X], HHLL); break;
	case 0x91: R[X] = MUL(R[X], R[Y]); break;
	case 0x92: R[Z] = MUL(R[X], R[Y]); break;

	case 0xA0: R[X] = DIV(R[X], HHLL); break;
	case 0xA1: R[X] = DIV(R[X], R[Y]); break;
	case 0xA2: R[Z] = DIV(R[X], R[Y]); break;
	case 0xA3: R[X] = MOD(R[X], HHLL); break;
	case 0xA4: R[X] = MOD(R[X], R[Y]); break;
	case 0xA5: R[Z] = MOD(R[X], R[Y]); break;
	case 0xA6: R[X] = REM(R[X], HHLL); break;
	case 0xA7: R[X] = REM(R[X], R[Y]); break;
	case 0xA8: R[Z] = REM(R[X], R[Y]); break;

	case 0xB0: R[X] = SHL(R[X], (LL & 0xF)); break; 
	case 0xB1: R[X] = SHR(R[X], (LL & 0xF)); break; 
	case 0xB2: R[X] = SAR(R[X], (LL & 0xF)); break; 
	case 0xB3: R[X] = SHL(R[X],       R[Y]); break;
	case 0xB4: R[X] = SHR(R[X],       R[Y]); break;
	case 0xB5: R[X] = SAR(R[X],       R[Y]); break;

	case 0xC0: STACK[SP++] = R[X]; break;
	case 0xC1: R[X] = STACK[--SP]; break;
	case 0xC2: memcpy(&STACK[SP], R, 32); SP += 16; break;
	case 0xC3: SP -= 16; memcpy(R, &STACK[SP], 32); break;
	case 0xC4: STACK[SP++] = chip16_getflags(); break;
	case 0xC5: chip16_setflags(STACK[--SP]); break;

	case 0xD0: chip16_load_palette(&MEM[HHLL]); break;
	case 0xD1: chip16_load_palette(&MEM[R[X]]); break;

	case 0xE0: R[X] = NOT(HHLL); break;
	case 0xE1: R[X] = NOT(R[X]); break;
	case 0xE2: R[X] = NOT(R[Y]); break;
	case 0xE3: R[X] = NEG(HHLL); break;
	case 0xE4: R[X] = NEG(R[X]); break;
	case 0xE5: R[X] = NEG(R[Y]); break;
	}
}

void  chip16_loop(double time)
{
	static double vid_60hz = 0;
	uint32_t op;

	VBLNK = 0;

	if(time >= vid_60hz)
	{
		chip16_update_display();
		VBLNK = 1;
		vid_60hz = time + 1.0 / 60.0;
	}

	op = *(uint32_t*)&MEM[PC]; PC += 4;

	chip16_decode(op);
}

void chip16_reset(void)
{
	memset(MEM, 0, MEMSIZE);
	memset(DISPLAY, 0, DISPSIZE);
	memset(DISPCOL, 0, DISPSIZE * 4);
	memset(R, 0, 32);

	BG = 0;

	PALETTE[0x0] = 0xFF000000;
	PALETTE[0x1] = 0xFF000000;
	PALETTE[0x2] = 0xFF888888;
	PALETTE[0x3] = 0xFF3239BF;
	PALETTE[0x4] = 0xFFAE7ADE;
	PALETTE[0x5] = 0xFF213D4C;
	PALETTE[0x6] = 0xFF255F90;
	PALETTE[0x7] = 0xFF5294E4;
	PALETTE[0x8] = 0xFF79D9EA;
	PALETTE[0x9] = 0xFF3B7A53;
	PALETTE[0xA] = 0xFF4AD5AB;
	PALETTE[0xB] = 0xFF382E25;
	PALETTE[0xC] = 0xFF7F4600;
	PALETTE[0xD] = 0xFFCCAB68;
	PALETTE[0xE] = 0xFFE4DEBC;
	PALETTE[0xF] = 0xFFFFFFFF;

	PC = SP = 0;
	SPRH = SPRW = 0;
	FLIPV = FLIPH = 0;

	snd_adrt(0, 0, 0, 0);
	snd_vol(0xF, 0xF); 
	snd_stop();
}

void chip16_load(void *rom, long size)
{
	char *h = (char*)rom;

	chip16_reset();

	if(h[0] == 'C' && h[1] == 'H' && h[2] == '1' && h[3] == '6')
	{
		size = *(uint32_t*)&h[0x06];
		PC   = *(uint16_t*)&h[0x0A];
		rom  = &h[16];
	}
	
	memcpy(MEM, rom, size);
}

void* chip16_getdisplay(void)
{
	return DISPCOL;
}

void chip16_init(void)
{
	//chip16_reset();
}

void chip16_shutdown(void)
{
	snd_stop();
}