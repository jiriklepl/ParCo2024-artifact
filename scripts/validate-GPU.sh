#!/bin/bash

set -eo pipefail

export USE_SLURM=${USE_SLURM:-0}

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

( cd PolyBenchGPU-Noarr && DATASET_SIZE=SMALL ./compare.sh )
( cd PolyBenchGPU-Noarr && DATASET_SIZE=MEDIUM ./compare.sh )
( cd PolyBenchGPU-Noarr && DATASET_SIZE=LARGE ./compare.sh )
( cd PolyBenchGPU-Noarr && DATASET_SIZE=EXTRALARGE ./compare.sh )
