/* Copyright (c) 2016 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#ifndef CHIP16_H
#define CHIP16_H

void* chip16_getdisplay(void);
void  chip16_loop(double time);
void  chip16_load(void *rom, long size);
void  chip16_input(int key, int state);

void  chip16_init(void);
void  chip16_shutdown(void);

#endif