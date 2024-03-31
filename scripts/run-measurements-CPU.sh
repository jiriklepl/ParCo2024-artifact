#!/bin/bash

export USE_SLURM=${USE_SLURM:-0}

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

( cd PolybenchC-Noarr && DATASET_SIZE=SMALL DATA_DIR=../results/PolybenchC/small-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=SMALL DATA_DIR=../results/PolybenchC-tuned/small-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=SMALL DATA_DIR=../results/PolybenchC-tbb/small-data ./collect.sh & wait )
( cd PolybenchC-Noarr-omp && DATASET_SIZE=SMALL DATA_DIR=../results/PolybenchC-omp/small-data ./collect.sh & wait )

( cd PolybenchC-Noarr && DATASET_SIZE=MEDIUM DATA_DIR=../results/PolybenchC/medium-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=MEDIUM DATA_DIR=../results/PolybenchC-tuned/medium-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=MEDIUM DATA_DIR=../results/PolybenchC-tbb/medium-data ./collect.sh & wait )
( cd PolybenchC-Noarr-omp && DATASET_SIZE=MEDIUM DATA_DIR=../results/PolybenchC-omp/medium-data ./collect.sh & wait )

( cd PolybenchC-Noarr && DATASET_SIZE=LARGE DATA_DIR=../results/PolybenchC/large-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=LARGE DATA_DIR=../results/PolybenchC-tuned/large-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=LARGE DATA_DIR=../results/PolybenchC-tbb/large-data ./collect.sh & wait )
( cd PolybenchC-Noarr-omp && DATASET_SIZE=LARGE DATA_DIR=../results/PolybenchC-omp/large-data ./collect.sh & wait )

( cd PolybenchC-Noarr && DATASET_SIZE=EXTRALARGE DATA_DIR=../results/PolybenchC/extralarge-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=EXTRALARGE DATA_DIR=../results/PolybenchC-tuned/extralarge-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=EXTRALARGE DATA_DIR=../results/PolybenchC-tbb/extralarge-data ./collect.sh & wait )
( cd PolybenchC-Noarr-omp && DATASET_SIZE=EXTRALARGE DATA_DIR=../results/PolybenchC-omp/extralarge-data ./collect.sh & wait )
