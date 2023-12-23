#!/bin/bash

export USE_SLURM=${USE_SLURM:-0}

( cd PolyBenchGPU-Noarr && ./collect.sh & wait )
