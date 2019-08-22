#!/bin/bash

DIR=$1

rm -f results/$DIR/*.png
cat results/$DIR/* | ./graph
mv Scalability.png results/$DIR/scalability.png
mv Speedup.png results/$DIR/speedup.png
