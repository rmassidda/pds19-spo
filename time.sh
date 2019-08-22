#!/bin/bash

SEED=42
N=4096
M=16
MAX_NW=$1
DELAY=$2
FF=$3

if [ -z "$1" ]
  then
    echo "Maximum nuber of workers required."
    exit
fi

if [ -z "$2" ]
  then
    echo "Delay in microseconds for the real function required."
    exit
fi

for ((i=0;i<=MAX_NW;i++)); do
  ./spo $SEED $N $M $i $DELAY $FF
done
