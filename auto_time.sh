#!/bin/bash

if [ "$#" -ne 5 ]; then
    echo "Usage: $0 n n_iter precision max_nw experiment_name"
    exit
fi

PREFIX=experiment
SEED=42
N=$1
M=$2
P=$3
MAXNW=$4
DIR=$5

mkdir -p $PREFIX
mkdir -p $PREFIX/$DIR

compute () {
  for ((i=0;i<=MAXNW;i++)); do
    ./spo $SEED $N $M $P $i $1
  done
}

echo "Computing C++ threads solution"
compute 2>$PREFIX/$DIR/cpp
echo "Computing FastFlow solution"
compute ff 2>$PREFIX/$DIR/ff
