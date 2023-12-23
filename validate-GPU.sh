#!/bin/bash

export USE_SLURM=${USE_SLURM:-0}

( cd PolyBenchGPU-Noarr && ./compare.sh & wait ) || exit 1
