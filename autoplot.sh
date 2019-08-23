#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 directory"
    exit
fi

DIR=$1

rm -f $DIR/*.png
cat $DIR/ff $DIR/cpp | ./plot
mv Scalability.png $DIR/scalability.png
mv Speedup.png $DIR/speedup.png
