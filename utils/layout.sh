#!/bin/bash


if [ $# -ne 2 ]
then
	echo "usage: $0 input_dir output_dir"
	exit 1
fi

test -e $2 || mkdir $2
for F in `(cd $1 && ls *.graphml)`;
do
	OUT=${F%.graphml}.dot
	graphml2gv $1/$F 2> /dev/null | dot -o $2/$OUT
done

