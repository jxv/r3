#! /bin/sh

cp shader/blit.vert shader/blit_alpha.vert
cp shader/blit.vert shader/gaussian.vert
cp shader/blit.vert shader/high_pass.vert

for i in `find shader | grep "\.frag" | grep -v "\.h"`
do
	echo "${i} => ${i}.h"
	xxd -i $i > $i.h
done

for i in `find shader | grep "\.vert" | grep -v "\.h"`
do
	echo "${i} => ${i}.h"
	xxd -i $i > $i.h
done
