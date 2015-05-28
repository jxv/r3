#! /bin/sh

for i in `find shader | grep "\.frag" | grep -v "\.h"`
do
	xxd -i $i > $i.h
done

for i in `find shader | grep "\.vert" | grep -v "\.h"`
do
	xxd -i $i > $i.h
done
