#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 max_nw directory"
    exit
fi

MAXNW=$1
DIR=$2

rm -f $DIR/*.png
cat $DIR/ff $DIR/cpp | ./plot $MAXNW
mv Scalability.png $DIR/scalability.png
mv Speedup.png $DIR/speedup.png
