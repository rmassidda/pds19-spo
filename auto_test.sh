#!/bin/bash

DIR=$1

mkdir results/$DIR

echo "CPP 0d"
./time.sh 64 0 2>results/$DIR/cpp_0d
echo "CPP 2d"
./time.sh 64 2 2>results/$DIR/cpp_2d
echo "FF 0d"
./time.sh 64 0 FF 2>results/$DIR/ff_0d
echo "FF 2d"
./time.sh 64 2 FF 2>results/$DIR/ff_2d
