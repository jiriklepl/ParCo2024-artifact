#!/bin/bash

set -eo pipefail

export USE_SLURM=${USE_SLURM:-0}

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

( cd PolyBenchGPU-Noarr && ./compare.sh & wait )
