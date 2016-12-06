/* Copyright (c) 2016 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "chip8.h"
#include "sound.h"
#include "video.h"

#include <stdint.h>
#include <memory.h>
#include <stdlib.h>

#define MEMSIZE  (32 * 1024 * 1024)
#define WIDTH    (256)
#define HEIGHT   (192)
#define DISPSIZE (WIDTH * HEIGHT)

static long     ROM_SIZE;

static uint8_t  MEM[MEMSIZE];      // 4kb memory CHIP8, 32mb for MEGA
static uint8_t  DISPLAY[DISPSIZE]; // CHIP8: 64x32, HIRES: 64x64, SUPER: 128x64, MEGA: 256x192

static uint32_t DISPCOL[DISPSIZE]; // Color out display
static uint32_t PALETTE[256];      // MEGA color palette

static uint8_t  CCOL;       // MEGA collision color index
static uint8_t  SPRW, SPRH; // MEGA sprite

static uint8_t  HIRES;      // Chip8 Hi-Res mode
static uint8_t  EXTENDED;   // Super Chip8 Extended mode
static uint8_t  MEGA;       // Mega Chip8 mode
						    
static uint32_t STACK[16];  // 16 levels nest
static uint32_t PC, I;      // Program Counter, I mem address.
						    
static uint8_t  HP48[8];    // 8  SUPER flags [0..7]
static uint8_t  V[16];      // 16 registers [0..F]
static uint8_t  INPUT[16];  // 16 keys [0..9, A..F]
						    
static uint8_t  DT; // Delay Timer, decrease at 60hz
static uint8_t  ST; // Sound Timer, decrease at 60hz
static uint8_t  SP; // Stack pointer

static int8_t   WAITKEY;
static int8_t   VBLNK;


uint8_t font5[] = 
			{0x60, 0xA0, 0xA0, 0xA0, 0xC0, // 0
			 0x40, 0xC0, 0x40, 0x40, 0xE0, // 1
			 0xC0, 0x20, 0x40, 0x80, 0xE0, // 2
			 0xC0, 0x20, 0x40, 0x20, 0xC0, // 3
			 0x20, 0xA0, 0xE0, 0x20, 0x20, // 4
			 0xE0, 0x80, 0xC0, 0x20, 0xC0, // 5
			 0x40, 0x80, 0xC0, 0xA0, 0x40, // 6
			 0xE0, 0x20, 0x60, 0x40, 0x40, // 7
			 0x40, 0xA0, 0x40, 0xA0, 0x40, // 8
			 0x40, 0xA0, 0x60, 0x20, 0x40, // 9
			 0x40, 0xA0, 0xE0, 0xA0, 0xA0, // A
			 0xC0, 0xA0, 0xC0, 0xA0, 0xC0, // B
			 0x60, 0x80, 0x80, 0x80, 0x60, // C
			 0xC0, 0xA0, 0xA0, 0xA0, 0xC0, // D
			 0xE0, 0x80, 0xC0, 0x80, 0xE0, // E
			 0xE0, 0x80, 0xC0, 0x80, 0x80};// F

uint16_t font10[] = 
			{0xC67C, 0xDECE, 0xF6D6, 0xC6E6, 0x007C, // 0
			 0x3010, 0x30F0, 0x3030, 0x3030, 0x00FC, // 1
			 0xCC78, 0x0CCC, 0x3018, 0xCC60, 0x00FC, // 2
			 0xCC78, 0x0C0C, 0x0C38, 0xCC0C, 0x0078, // 3
			 0x1C0C, 0x6C3C, 0xFECC, 0x0C0C, 0x001E, // 4
			 0xC0FC, 0xC0C0, 0x0CF8, 0xCC0C, 0x0078, // 5
			 0x6038, 0xC0C0, 0xCCF8, 0xCCCC, 0x0078, // 6
			 0xC6FE, 0x06C6, 0x180C, 0x3030, 0x0030, // 7
			 0xCC78, 0xECCC, 0xDC78, 0xCCCC, 0x0078, // 8
			 0xC67C, 0xC6C6, 0x0C7E, 0x3018, 0x0070, // 9
			 0x7830, 0xCCCC, 0xFCCC, 0xCCCC, 0x00CC, // A
			 0x66FC, 0x6666, 0x667C, 0x6666, 0x00FC, // B
			 0x663C, 0xC0C6, 0xC0C0, 0x66C6, 0x003C, // C
			 0x6CF8, 0x6666, 0x6666, 0x6C66, 0x00F8, // D
			 0x62FE, 0x6460, 0x647C, 0x6260, 0x00FE, // E
			 0x66FE, 0x6462, 0x647C, 0x6060, 0x00F0};// F

void chip8_input(int key, int state)
{
	switch(key)
	{
	case 0x58: INPUT[0x0] = state & 1; break; // X
	case 0x31: INPUT[0x1] = state & 1; break; // 1
	case 0x32: INPUT[0x2] = state & 1; break; // 2
	case 0x33: INPUT[0x3] = state & 1; break; // 3
	case 0x51: INPUT[0x4] = state & 1; break; // Q
	case 0x57: INPUT[0x5] = state & 1; break; // W
	case 0x45: INPUT[0x6] = state & 1; break; // E
	case 0x41: INPUT[0x7] = state & 1; break; // A
	case 0x53: INPUT[0x8] = state & 1; break; // S
	case 0x44: INPUT[0x9] = state & 1; break; // D
	case 0x5A: INPUT[0xA] = state & 1; break; // Z
	case 0x43: INPUT[0xB] = state & 1; break; // C
	case 0x34: INPUT[0xC] = state & 1; break; // 4
	case 0x52: INPUT[0xD] = state & 1; break; // R
	case 0x46: INPUT[0xE] = state & 1; break; // F
	case 0x56: INPUT[0xF] = state & 1; break; // V
	}
};

void chip8_update_display(void)
{
	int i, index;

	if(MEGA)          video_disp_res(256, 192);
	else if(EXTENDED) video_disp_res(128, 64);
	else if(HIRES)    video_disp_res(64, 64);
	else              video_disp_res(64, 32);


	for(i = 0; i < DISPSIZE; i++)
	{
		index = DISPLAY[i];
		DISPCOL[i] = PALETTE[index];
	}
}

void chip8_draw_mega(uint32_t x, uint32_t y, uint32_t n)
{
	uint32_t i, j, nx, ny;

	const uint32_t h = SPRH == 0 ? 256 : SPRH;
	const uint32_t w = SPRW == 0 ? 256 : SPRW;
	
	uint8_t pix, *d, *mem = &MEM[I];

	V[0xF] = 0;

	for(j = 0; j < h; j++)
	{
		ny  = (y + j);
		if(ny >= HEIGHT) break;

		d  = &DISPLAY[ny * WIDTH];
		
		for(i = 0; i < w; i++)
		{
			nx  = (x + i);
			pix = *(mem++);
			
			if(nx < WIDTH)
			{
				if(d[nx] == CCOL) V[0xF] = 1;
				if(pix) d[nx] = pix;
			}			
		}
	}
}

void chip8_draw(uint32_t x, uint32_t y, uint32_t n)
{
	uint32_t i, j, nx, ny, xx, yy, nnx, nny;
	uint16_t line;
	uint8_t *d, *dd, bit;

	uint8_t *mem = &MEM[I];

	const uint32_t width = EXTENDED ? 128 : 64;
	const uint32_t height = (EXTENDED | HIRES) ? 64 : 32;

	const uint32_t sx = 256 / width;
	const uint32_t sy = 192 / height;

	uint32_t h = (n == 0) ? 16 : n;
	uint32_t w = (EXTENDED && h == 16)? 16 : 8;

	y &= height - 1;
	x &= width - 1;

	V[0xF] = 0;

	for(j = 0; j < h; j++)
	{
		ny  = (y + j) * sy;
		if(ny >= HEIGHT) break;

		d  = &DISPLAY[ny * WIDTH];
		line = *(mem++);

		if(w == 16)
			line = line << 8 | *(mem++);

		for(i = 0; i < w; i++)
		{
			nx  = (x + i) * sx;
			if(nx >= WIDTH) break;

			bit = (line >> (w - i - 1)) & 1;
			V[0xF] |= d[nx] & bit;
			d[nx] ^= bit;

			for(yy = 0; yy < sy; yy++)
			{
				nny = ny + yy;
				dd  = &DISPLAY[nny * WIDTH];

				for(xx = 0; xx < sx; xx++)
				{
					nnx = nx + xx;
					dd[nnx] = d[nx];
				}
			}
			
		}
	}

	// set dirty display
}

void chip8_scroll_right(void)
{
	uint32_t dx, dy, dd;
	uint8_t *d;

	for(dy = 0; dy < HEIGHT; dy++)
	{
		d = &DISPLAY[dy * WIDTH];

		for(dx = WIDTH - 8; dx >= 8; dx -= 8)
		for(dd = 0; dd < 8; dd++)
		{
			d[dx + dd] = d[dx - 8 + dd];
		}

		memset(d, 0, 8);
	}
}

void chip8_scroll_left(void)
{
	uint32_t dx, dy, dd;
	uint8_t *d;

	for(dy = 0; dy < HEIGHT; dy++)
	{
		d = &DISPLAY[dy * WIDTH];

		for(dx = 0; dx < WIDTH - 8; dx += 8)
		for(dd = 0; dd < 8; dd++)
		{
			d[dx + dd] = d[dx + 8 + dd];
		}

		memset(&d[dx], 0, 8);
	}
}

void chip8_scroll_up(uint16_t op)
{
}

void chip8_scroll_down(uint16_t op)
{
	int i, n = (op & 0xF) * 3;
	int block = WIDTH * 3;
				
	for(i = (HEIGHT - 3) * WIDTH; i > n; i -= block)
		memcpy(&DISPLAY[i], &DISPLAY[i - block], block);

	for(; i >= 0; i -= block)
		memset(&DISPLAY[i], 0, block);
}

void chip8_decode(uint16_t op)
{
	uint8_t x  = (op >> 8) & 0xF;
	uint8_t y  = (op >> 4) & 0xF;
	uint8_t nn = op & 0xFF;

	switch(op >> 12)
	{
	case 0x0:
		switch(x)
		{
		case 0x0:
			switch(nn)
			{
			case 0x00: PC -= 2; break; // zero loop

			case 0x10: MEGA = 0; break;
			case 0x11: MEGA = 1; break;

			case 0xE0:
				if(MEGA)
				{
					if(VBLNK)
					{
						chip8_update_display();
						memset(DISPLAY, 0, DISPSIZE);
					}
					else
						PC -= 2;
				}
				else
					memset(DISPLAY, 0, DISPSIZE);
				
				break;
			
			case 0xEE:
				PC = STACK[SP];
				SP = (SP - 1) & 0xF;
				break;

			case 0xFB: chip8_scroll_right(); break;
			case 0xFC: chip8_scroll_left(); break;

			case 0xFE: EXTENDED = 0; break;
			case 0xFF: EXTENDED = 1; break;

			default:
				if(y == 0xB) chip8_scroll_up(op);
				if(y == 0xC) chip8_scroll_down(op);
				break;
			}	
			break;

		case 0x1:
			I  = nn << 16;
			I |= MEM[PC++] << 8;
			I |= MEM[PC++];
			break;
		
		case 0x2:
			if(HIRES && nn == 0x30)
				memset(DISPLAY, 0, DISPSIZE);
			else
			{
				uint32_t i;
				uint32_t *m = (uint32_t*)&MEM[I];

				for(i = 0; i <= nn; i++) // 0xARGB to 0xRGBA
					PALETTE[(i+1) & 0xFF] = m[i] >> 8 | (0xFF) << 24;
			}			
			break;

		case 0x3: SPRW = nn; break;
		case 0x4: SPRH = nn; break;
		case 0x5: break;
		case 0x6: snd_pcm8(&MEM[I], ROM_SIZE - I); break;
		case 0x7: break;
		case 0x8: break;
		case 0x9: CCOL = nn; break;
		};
		break;

	case 0x1:
		if(PC == 0x202 && op == 0x1260)
		{
			HIRES = 1;
			op = 0x12C0;
		}

		PC = op & 0xFFF;
		break;
	
	case 0x2:
		SP = (SP + 1) & 0xF;
		STACK[SP] = PC;
		PC = op & 0xFFF;
		break;

	case 0x3: if(V[x] == nn) PC += 2; break;
	case 0x4: if(V[x] != nn) PC += 2; break;
	case 0x5: if(V[x] == V[y]) PC += 2; break;

	case 0x6: V[x]  = nn; break;
	case 0x7: V[x] += nn; break;

	case 0x8:
		{
			const uint8_t f = 0xF;

			switch(op & 0xF)
			{
			case 0x0: V[x]  = V[y]; break;
			case 0x1: V[x] |= V[y]; break;
			case 0x2: V[x] &= V[y]; break;
			case 0x3: V[x] ^= V[y]; break;
			
			case 0x4:
				V[f] = (V[x] + V[y]) > 0xFF ? 1 : 0;
				V[x] = (V[x] + V[y]) & 0xFF;
				break;

			case 0x5:
				V[f] = V[x] >= V[y] ? 1 : 0;
				V[x] = (V[x] - V[y]) & 0xFF;
				break;

			case 0x6:
				V[f] = V[x] & 1;
				V[x] = V[x] >> 1;
				break;

			case 0x7:
				V[f] = V[y] >= V[x] ? 1 : 0;
				V[x] = (V[y] - V[x]) & 0xFF;
				break;

			case 0x8: break;
			case 0x9: break;
			case 0xA: break;
			case 0xB: break;
			case 0xC: break;
			case 0xD: break;

			case 0xE:
				V[f] = V[x] >> 7;
				V[x] = V[x] << 1;
				break;

			case 0xF: break;
			}
		}
		break;

	case 0x9: if(V[x] != V[y]) PC += 2; break;
	case 0xA: I = op & 0xFFF; break;
	case 0xB: PC = (op & 0xFFF) + V[0]; break;
	case 0xC: V[x] = rand() & (op & 0xFF); break;

	case 0xD:
		if(MEGA) chip8_draw_mega(V[x], V[y], op & 0xF);
		else chip8_draw(V[x], V[y], op & 0xF);
		break;

	case 0xE:
		switch(nn)
		{
		case 0x9E: if(INPUT[V[x] & 0xF] != 0) PC += 2; break;
		case 0xA1: if(INPUT[V[x] & 0xF] == 0) PC += 2; break;
		}
		break;

	case 0xF:
		switch(nn)
		{
		case 0x07: V[x] = DT; break;
		case 0x0A: WAITKEY = x + 1; break;
		case 0x15: DT = V[x]; break;
		case 0x18: ST = V[x]; break;
		//case 0x18: snd_beep(800, V[x]); break;

		case 0x1E:
			V[0xF] = (V[x] + I) > 0xFFF ? 1 : 0;
			I = (I + V[x]) & (MEGA ? 0xFFFFFF : 0xFFF);
			break;

		case 0x29: I = (V[x] & 0xF) * 5; break;
		case 0x30: I = (V[x] & 0xF) * 10 + 80; break;

		case 0x33:
			{
				int d3 = V[x] / 100;
				int d2 = (V[x] - d3 * 100) / 10;
				int d1 = V[x] - d2 * 10 - d3 * 100;

				MEM[I + 0] = d3 & 0xFF;
				MEM[I + 1] = d2 & 0xFF;
				MEM[I + 2] = d1 & 0xFF;
			}
			break;

		case 0x55: memcpy(&MEM[I], V, x + 1); break;
		case 0x65: memcpy(V, &MEM[I], x + 1); break;
		case 0x75: memcpy(HP48, V, (x & 7) + 1); break;
		case 0x85: memcpy(V, HP48, (x & 7) + 1); break;
		}
		break;
	}

}

void chip8_loop(double time)
{
	static double tick_cpu = 0;
	static double tick_60hz = 0;
	uint16_t op;

	VBLNK = 0;

	if(time >= tick_60hz)
	{
		VBLNK = 1;

		if(DT > 0) DT--;
		
		if(ST > 0)
		{
			ST--;
			snd_beep(800, 20);
		}	

		if(!MEGA)
			chip8_update_display();

		tick_60hz = time + 1.0 / 60.0;
	}

	if(WAITKEY)
	{
		int i;

		for(i = 0; i < 16; i++)
		{
			if(INPUT[i] != 0)
			{
				V[WAITKEY - 1] = i & 0xF;
				INPUT[i] = 0;
				WAITKEY = 0;
			}
		}
	}
	else if(MEGA || time >= tick_cpu)
	{
		op  = MEM[PC++] << 8;
		op |= MEM[PC++];

		chip8_decode(op);

		tick_cpu = time + 1.0 / 1000.0;
	}
	
}

void chip8_reset(void)
{
	int i;

	memset(MEM, 0, MEMSIZE);
	memset(DISPLAY, 0, DISPSIZE);
	memset(DISPCOL, 0, DISPSIZE * 4);
	
	memset(PALETTE, 0, sizeof(PALETTE));

	memset(HP48, 0, sizeof(HP48));
	memset(V, 0, sizeof(V));
	memset(INPUT, 0, sizeof(INPUT));
	memset(STACK, 0, sizeof(STACK));

	memcpy(MEM, font5, sizeof(font5));
	memcpy(MEM + 80, font10, sizeof(font10));

	for(i = 0; i < 8; i++) HP48[i] = rand() & 0x3F;

	PALETTE[0] = 0xFF0F0F0F; // BG
	PALETTE[1] = 0xFFFFE1CD; // FG

	PC = I = 0x200;
	SP = DT = ST = 0;

	MEGA = HIRES = EXTENDED = 0;
	WAITKEY = 0;

	snd_stop();
}

void chip8_load(void *rom, long size)
{
	chip8_reset();
	memcpy(&MEM[0x200], rom, size);
	ROM_SIZE = size;
}

void* chip8_getdisplay(void)
{
	return DISPCOL;
}

void chip8_init(void)
{
	//chip8_reset();
}

void chip8_shutdown(void)
{
	snd_stop();
	snd_pcm8(NULL, 0);
}