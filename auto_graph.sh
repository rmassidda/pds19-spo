#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: ./auto_test.sh max_nw directory"
    exit
fi

MAXNW=$1
DIR=$2

rm -f $DIR/*.png
cat $DIR/* | ./graph $MAXNW
mv Scalability.png $DIR/scalability.png
mv Speedup.png $DIR/speedup.png
