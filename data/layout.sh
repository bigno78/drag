#!/bin/sh

test -e $2 || mkdir $2
for file in $(ls $1)
do
	dot -Tsvg -o $2/$file.svg $1/$file
done

