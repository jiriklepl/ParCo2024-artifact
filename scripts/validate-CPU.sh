#!/bin/bash

export USE_SLURM=${USE_SLURM:-0}

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

( cd PolybenchC-Noarr && DATASET_SIZE=SMALL ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=SMALL ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=SMALL ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-omp && DATASET_SIZE=SMALL ./compare.sh & wait ) || exit 1

( cd PolybenchC-Noarr && DATASET_SIZE=MEDIUM ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=MEDIUM ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=MEDIUM ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-omp && DATASET_SIZE=MEDIUM ./compare.sh & wait ) || exit 1

( cd PolybenchC-Noarr && DATASET_SIZE=LARGE ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=LARGE ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=LARGE ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-omp && DATASET_SIZE=LARGE ./compare.sh & wait ) || exit 1

( cd PolybenchC-Noarr && DATASET_SIZE=EXTRALARGE ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=EXTRALARGE ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=EXTRALARGE ./compare.sh & wait ) || exit 1
( cd PolybenchC-Noarr-omp && DATASET_SIZE=EXTRALARGE ./compare.sh & wait ) || exit 1
