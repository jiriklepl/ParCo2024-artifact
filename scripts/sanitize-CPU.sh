#!/bin/bash -ex

export USE_SLURM=${USE_SLURM:-0}

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

dirname=$(mktemp -d)

cleanup() {
    rm -rf "$dirname"
}

trap cleanup EXIT

run_compares() {
    ( cd PolybenchC-Noarr && ./compare.sh )
    ( cd PolybenchC-Noarr-tuned && ./compare.sh )
    ( cd PolybenchC-Noarr-tbb && ./compare.sh )
    ( cd PolybenchC-Noarr-omp && ./compare.sh )
}

export DATASET_SIZE=SMALL
export DATA_DIR="$dirname"
export NUM_RUNS=1

OLD_CMAKE_CXX_FLAGS=$CMAKE_CXX_FLAGS

export CMAKE_CXX_FLAGS="-fsanitize=address -g -O0 $OLD_CMAKE_CXX_FLAGS"
run_compares

export CMAKE_CXX_FLAGS="-fsanitize=address -g -O2 $OLD_CMAKE_CXX_FLAGS"
run_compares

export CMAKE_CXX_FLAGS="-fsanitize=undefined -g -O0 $OLD_CMAKE_CXX_FLAGS"
run_compares

export CMAKE_CXX_FLAGS="-fsanitize=undefined -g -O2 $OLD_CMAKE_CXX_FLAGS"
run_compares

export CMAKE_CXX_FLAGS="-fsanitize=leak -g -O0 $OLD_CMAKE_CXX_FLAGS"
run_compares

export CMAKE_CXX_FLAGS="-fsanitize=leak -g -O2 $OLD_CMAKE_CXX_FLAGS"
run_compares
