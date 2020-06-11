#!/bin/bash

OUT=opt.txt
IN=data/

# compile the helper program for calculating total edge length
g++ -std=c++17 optimality.cpp -o counter

# clear the output file
echo -n > $OUT

# process all DOT files
for F in `(cd $IN && ls *.gv)`; do
	dot -o out.dot $IN/$F
	L=$(./counter out.dot)
	echo $F $L >> $OUT
done

# cleanup
test -e out.dot && rm out.dot
rm counter

