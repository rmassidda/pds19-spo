#!/bin/bash

if [ "$#" -ne 5 ]; then
    echo "Usage: ./auto_test.sh n n_iter precision max_nw destination"
    exit
fi

SEED=42
N=$1
M=$2
P=$3
MAXNW=$4
DIR=$5

mkdir -p results
mkdir -p results/$DIR

compute () {
  for ((i=0;i<=MAXNW;i++)); do
    ./spo $SEED $N $M $P $i $1
  done
}

echo "Computing C++ threads solution"
compute 2>results/$DIR/cpp
echo "Computing FastFlow solution"
compute ff 2>results/$DIR/ff
