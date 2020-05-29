#!/bin/bash

java -jar DAGmar.jar -n $1 -e $2 -c -f g.tmp
graphml2gv g.tmp.graphml -o $3/g.$1n.$2e.gv
rm g.tmp.graphml

