all:
	cc main.c -o cube -std=c11 `sdl2-config --cflags --libs` ../libr3.a -lc -lm -lGLESv2 -D_GNU_SOURCE -ffast-math -lml

gcw0:
	/opt/gcw0-toolchain/usr/bin/mipsel-linux-cc main.c -o cube -std=c11 -I/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include/SDL2 -D_REENTRANT -L/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/lib -lc -lm -lSDL2 -lpthread -lGLESv2 -lr3 -lml -D_GNU_SOURCE -ffast-math -DGCW0
