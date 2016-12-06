/* Copyright (c) 2016 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#ifndef SOUND_H
#define SOUND_H

void snd_pcm8(void *mem, long size);
void snd_beep(int freq, int ms);
void snd_stop(void);

void snd_adrt(int a, int d, int r, int t);
void snd_vol(int v, int s);

int  snd_init(void *hwnd);
void snd_shutdown(void);

void snd_proc(unsigned int msg, void *data);

#endif