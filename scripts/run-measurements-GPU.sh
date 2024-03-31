#!/bin/bash

export USE_SLURM=${USE_SLURM:-0}

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

( cd PolyBenchGPU-Noarr && DATA_DIR=../results/PolyBenchGPU/data ./collect.sh & wait )
