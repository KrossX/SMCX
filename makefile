CC = @gcc
CFLAGS = -Wall -O2 -flto -march=i686 -m32 -mwindows -s -std=c11
LDLIBS = -lopengl32 -lwinmm

%.o : smcx/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

default: smcx.o chip8.o chip16.o sound.o video.o
	$(CC) $^ -o smcx.exe $(CFLAGS) $(LDLIBS)
	