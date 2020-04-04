#!/bin/bash

#set -x

prefix=../cross_data/$1_$2

if test -e $prefix/
then
	rm $prefix/$2/*
	for a in  gv out dot 
	do
		test "$(ls -A $prefix/$a/)" && rm $prefix/$a/*
	done
else
	mkdir $prefix/
	for a in gv out dot 
	do
		mkdir $prefix/$a/
	done
fi

java -jar DAGmar.jar multi -n $1 -d $2 -i "1 to 50" -c -f cross $prefix
sh convert.sh $prefix/$2 $prefix/gv

