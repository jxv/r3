all:
	cc *.c -o demo -std=c11 `sdl2-config --cflags --libs` -lr3 -lc -lm -lSDL2_test -lSDL2_mixer -lGLESv2 -D_GNU_SOURCE -ffast-math -lml -DHAVEOPENGL -DHAVEOPENGLES2 -DHAVEOPENGLES

gcw0:
	/opt/gcw0-toolchain/usr/bin/mipsel-linux-cc *.c -o demo -std=c11 -I/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include/SDL2 -D_REENTRANT -L/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/lib -lc -lm -lSDL2_test -lSDL2_mixer -lSDL2 -lpthread -lGLESv2 -lr3 -lml -D_GNU_SOURCE -ffast-math -DGCW0
