/* Copyright (c) 2016 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "sound.h"

#include <windows.h>
#include <math.h>

#pragma comment(lib, "winmm.lib")

#define WAVE_NUMBUFF    (5)
#define WAVE_BUFFSIZE   (256)
#define WAVE_SAMPLERATE (11025)
#define WAVE_1MS        (WAVE_SAMPLERATE / 1000)

#define PI (3.1415926535897932384626433832795)

HWAVEOUT waveout;
WAVEHDR  waveh[WAVE_NUMBUFF];
BYTE     waveb[WAVE_NUMBUFF][WAVE_BUFFSIZE];

enum
{
	MODE_NONE,
	MODE_ATTACK,
	MODE_DECAY,
	MODE_SUSTAIN,
	MODE_RELEASE
};

struct wave
{
	int freq;
	int mode;
	int duration;
} WAVE[256];

int WAVE_READ;
int WAVE_WRITE;

int WAVE_ATTACK;
int WAVE_DECAY;
int WAVE_RELEASE;

int WAVE_VOLUME;
int WAVE_SUSTAIN;

BYTE *MEGA_SND = NULL;
long MEGA_SIZE, MEGA_POS;

double snd_triangle(double angle)
{
	return angle < PI ? 2 * angle / PI - 1 : 1 - 2 * (angle - PI) / PI;
};

double snd_sawtooth(double angle)
{
	return angle/PI - 1.0;
};

double snd_square(double angle)
{
	return angle < PI ? -1 : 1;
};

double snd_noise(double angle)
{
	return 2.0 * rand() / (double)(RAND_MAX+1) - 1.0;
};

typedef double (*wave_func_t)(double angle);
wave_func_t wave_func = snd_triangle;

void wave_fill(BYTE *buff)
{
	static double angle = 0;
	static double freq = 0;
	
	static double vol  = 0, vinc = 0;
	static int mode = 0, step = 0;

	static struct wave *w = &WAVE[0];
	
	int i;

	for(i = 0; i < WAVE_BUFFSIZE; i++)
	{
		while(WAVE_READ != WAVE_WRITE)
		{
			w = &WAVE[WAVE_READ++];
			WAVE_READ &= 0xFF;

			freq = w->freq;
			mode = w->mode;

			if(mode == MODE_ATTACK)
			{
				vol  = 0;
				vinc = WAVE_ATTACK ? WAVE_VOLUME / (double)WAVE_ATTACK : 0;
				step = WAVE_ATTACK;
			}
		}

		switch(mode)
		{
		case MODE_NONE:
			buff[i] = 0x80;
			continue;
			break;

		case MODE_ATTACK:
			if(step <= 0)
			{
				vol  = WAVE_VOLUME;
				vinc = WAVE_DECAY ? (WAVE_SUSTAIN - WAVE_VOLUME) / (double)WAVE_DECAY : 0;
				step = WAVE_DECAY;
				mode = MODE_DECAY;
			}
			break;

		case MODE_DECAY:
			if(step <= 0)
			{
				vol  = WAVE_SUSTAIN;
				vinc = 0;
				step = w->duration;
				mode = MODE_SUSTAIN;
			}
			break;

		case MODE_SUSTAIN:
			if(step <= 0)
			{
				vol  = WAVE_SUSTAIN;
				vinc = WAVE_RELEASE ? -WAVE_SUSTAIN / (double)WAVE_RELEASE : 0;
				step = WAVE_RELEASE;
				mode = MODE_RELEASE;
			}
			break;

		case MODE_RELEASE:
			if(step <= 0)
			{
				vol  = 0;
				vinc = 0;
				mode = MODE_NONE;
			}
			break;
		}

		buff[i] = (BYTE)(0x80 + vol * wave_func(angle));

		vol += vinc;

		step--;

		angle += (2*PI*freq) / WAVE_SAMPLERATE;
		angle -= (int)(angle / (2*PI)) * (2*PI);
		//while(angle >= 2*PI) angle -= 2*PI;
	}

	if(MEGA_SND != NULL)
	{
		for(i = 0; i < WAVE_BUFFSIZE; i++)
		{
			buff[i] = buff[i] + MEGA_SND[MEGA_POS] - 0x80;

			MEGA_POS++; 
			if(MEGA_POS == MEGA_SIZE) MEGA_POS = 0;
		}
	}
}

void snd_proc(unsigned int msg, void *data)
{
	int i;

	switch(msg)
	{
	case MM_WOM_OPEN:
		for(i = 0; i < WAVE_NUMBUFF; i++)
		{
			memset(&waveh[i], 0, sizeof(WAVEHDR));

			waveh[i].dwBufferLength = WAVE_BUFFSIZE;
			waveh[i].lpData = (LPSTR)waveb[i];

			waveOutPrepareHeader(waveout, &waveh[i], sizeof(WAVEHDR));
				
			wave_fill(waveb[i]);
			waveOutWrite(waveout, &waveh[i], sizeof(WAVEHDR));
		}		
		break;

	case MM_WOM_CLOSE:
		for(i = 0; i < WAVE_NUMBUFF; i++)
		{
			waveOutUnprepareHeader(waveout, &waveh[i], sizeof(WAVEHDR));
		}
		break;

	case MM_WOM_DONE:
		wave_fill((BYTE*)((LPWAVEHDR)data)->lpData);
		waveOutWrite(waveout, (LPWAVEHDR)data, sizeof(WAVEHDR));
		break;
	}
}

int snd_init(void *hwnd)
{
	static WAVEFORMATEX wave_fmt;

	wave_fmt.cbSize          = 0;
	wave_fmt.nAvgBytesPerSec = WAVE_SAMPLERATE;// * (8 / 8) * (1); // samplerate * (bps/8) * channels
	wave_fmt.nBlockAlign     = 1;
	wave_fmt.nChannels       = 1;
	wave_fmt.nSamplesPerSec  = WAVE_SAMPLERATE;
	wave_fmt.wBitsPerSample  = 8;
	wave_fmt.wFormatTag      = WAVE_FORMAT_PCM;

	WAVE_ATTACK  = 0 * WAVE_1MS;
	WAVE_DECAY   = 0 * WAVE_1MS;
	WAVE_RELEASE = 50 * WAVE_1MS;

	WAVE_VOLUME  = 0x7F;
	WAVE_SUSTAIN = 0x7F;

	snd_stop();

	if(waveOutOpen(&waveout, WAVE_MAPPER, &wave_fmt, (DWORD)hwnd, 0, CALLBACK_WINDOW) != MMSYSERR_NOERROR)
	{
		return 1;
	}

	return ERROR_SUCCESS;
}

void snd_adrt(int a, int d, int r, int t)
{
	const int attack_ms[16] = {2, 8, 16, 24, 38, 56, 68, 80, 100, 250, 500, 800, 1000, 3000, 5000, 8000};
	const int decay_ms[16] = {6, 24, 48, 72, 114, 168, 204, 240, 300, 750, 1500, 2400, 3000, 9000, 15000, 24000};

	WAVE_ATTACK  = attack_ms[a] * WAVE_1MS;
	WAVE_DECAY   = decay_ms[d]  * WAVE_1MS;
	WAVE_RELEASE = decay_ms[r]  * WAVE_1MS;

	switch(t & 0xF)
	{
	case 1:  wave_func = snd_sawtooth; break;
	case 2:  wave_func = snd_square; break;
	case 3:  wave_func = snd_noise; break;
	default: wave_func = snd_triangle; break;
	}	
}

void snd_vol(int v, int s)
{
	WAVE_VOLUME = v * 0x8;
	WAVE_SUSTAIN = s * 0x8;
}

void snd_shutdown(void)
{
	waveOutReset(waveout);
	waveOutClose(waveout);
	waveout = NULL;
}

void snd_beep(int freq, int ms)
{
	WAVE[WAVE_WRITE].mode = MODE_ATTACK;
	WAVE[WAVE_WRITE].freq = freq;
	WAVE[WAVE_WRITE].duration = ms * WAVE_1MS - WAVE_ATTACK - WAVE_DECAY;
	WAVE_WRITE = (WAVE_WRITE + 1) & 0xFF;
}

void snd_stop(void)
{
	WAVE[WAVE_WRITE].mode = MODE_NONE;
	WAVE_WRITE = (WAVE_WRITE + 1) & 0xFF;
}

void snd_pcm8(void *mem, long size)
{
	MEGA_POS = 0;
	MEGA_SIZE = size;
	MEGA_SND = (BYTE*)mem;
}


//void beep_play(void)
//{
//	if(!waveout) snd_init();
//
//	//PlaySoundA("beep.wav", instance, SND_ASYNC | SND_LOOP | SND_FILENAME);
//}
//
//void beep_stop(void)
//{
//	snd_shutdown();
//	//PlaySoundA(NULL, NULL, SND_PURGE);
//}