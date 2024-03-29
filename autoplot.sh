#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 directory"
    exit
fi

DIR=$1

./plot $DIR/map-reduce.rst $DIR/FastFlow.rst
mv Scalability.png $DIR/scalability.png
mv Speedup.png $DIR/speedup.png
mv Efficiency.png $DIR/efficiency.png
