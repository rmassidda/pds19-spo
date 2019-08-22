#!/bin/bash

DIR=$1
PREC=$2

mkdir results/$DIR

echo "CPP"
./time.sh 128 $PREC 2>results/$DIR/cpp
echo "FF"
./time.sh 128 $PREC FF 2>results/$DIR/ff
