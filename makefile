all:
	xxd -i shader/cell.frag > shader/cell.frag.h
	xxd -i shader/minimum.frag > shader/minimum.frag.h
	xxd -i shader/normal.vert > shader/normal.vert.h
	xxd -i shader/color.vert > shader/color.vert.h
	xxd -i shader/color.frag > shader/color.frag.h
	xxd -i shader/texture.vert > shader/texture.vert.h
	xxd -i shader/texture.frag > shader/texture.frag.h
	gcc src/r3.c -c -o src/r3.o -I./include -Wall -Werror -pedantic -std=c11 -ffast-math -g -O3 -lml -lm -lc -D_GNU_SOURCE
	gcc src/spec.c -c -o src/spec.o -I./include -Wall -Werror -pedantic -std=c11 -ffast-math -g -O3 -lml -lm -lc -D_GNU_SOURCE
	ar rvs libr3.a src/*.o
clean:
	rm libr3.a src/*.o
install:
	cp include/*.h /usr/include
	cp libr3.a /usr/lib/

gcw0:
	mipsel-gcw0-linux-uclibc-cc src/r3.c -c -o src/r3.o -I./include -Wall -Werror -pedantic -std=c11 -ffast-math -g -O2 -lc -lm -D_GNU_SOURCE -I./include
	mipsel-gcw0-linux-uclibc-ar rvs libr3.a src/*.o
install_gcw0:
	mkdir -p /opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include
	cp include/*.h /opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include
	cp libr3.a /opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/lib
