#!/bin/bash

DIR=$1

cat results/$DIR/*_0d | ./graph
mv Scalability.png results/sc_$DIR\_0d.png
mv Speedup.png results/sp_$DIR\_0d.png

cat results/$DIR/*_2d | ./graph
mv Scalability.png results/sc_$DIR\_2d.png
mv Speedup.png results/sp_$DIR\_2d.png
