/* Copyright (c) 2016 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#ifndef CHIP8_H
#define CHIP8_H

void* chip8_getdisplay(void);
void  chip8_loop(double time);
void  chip8_load(void *rom, long size);
void  chip8_input(int key, int state);

void  chip8_init(void);
void  chip8_shutdown(void);

#endif