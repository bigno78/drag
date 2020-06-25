#!/bin/bash

test -e $2 || mkdir $2
for file in `(cd $1 && ls *.graphml)`;
do
	OUT=${file%.graphml}.gv
	graphml2gv $1/$file -o $2/$OUT 2> /dev/null
	test $1 = $2 && rm $1/$file
done

