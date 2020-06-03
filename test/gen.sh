#!/bin/bash

OUT=opt.txt
IN=data

# compile the helper program for calculating total edge length
g++ -std=c++17 optimality.cpp

# clear the output file
echo -n > $OUT

# process all DOT files
for F in $(ls $IN/*.gv); do
	dot -o out.dot $F
	L=$(./a.out out.dot)
	echo $F $L >> $OUT
done

# cleanup
rm out.dot

