all:
	gcc src/r3.c -c -o src/re.o -I./include -Wall -Werror -pedantic -std=c11 -ffast-math -g -O3 -lc -lm -lml -D_GNU_SOURCE
	ar rvs libr3.a src/re.o
clean:
	rm libr3.a src/re.o
install:
	cp include/*.h /usr/include
	cp libr3.a /usr/lib/

gcw0:
	mipsel-gcw0-linux-uclibc-cc src/r3.c -c -o src/re.o -I./include -Wall -Werror -pedantic -std=c11 -ffast-math -g -O2 -lc -lm -D_GNU_SOURCE -I./include
	mipsel-gcw0-linux-uclibc-ar rvs libr3.a src/*.o
install_gcw0:
	mkdir -p /opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include
	cp include/*.h /opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include
	cp libr3.a /opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/lib
