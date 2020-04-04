#!/bin/sh

test -e $2 || mkdir $2
for file in $(ls $1)
do
	graphml2gv $1/$file -o $2/$file.gv
done

