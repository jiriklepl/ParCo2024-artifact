#!/bin/bash

export USE_SLURM=${USE_SLURM:-0}

( cd PolybenchC-Noarr && DATASET_SIZE=MEDIUM DATA_DIR=medium-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=MEDIUM DATA_DIR=medium-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=MEDIUM DATA_DIR=medium-data ./collect.sh & wait )

( cd PolybenchC-Noarr && DATASET_SIZE=LARGE DATA_DIR=large-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=LARGE DATA_DIR=large-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=LARGE DATA_DIR=large-data ./collect.sh & wait )

( cd PolybenchC-Noarr && DATASET_SIZE=EXTRALARGE DATA_DIR=extralarge-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tuned && DATASET_SIZE=EXTRALARGE DATA_DIR=extralarge-data ./collect.sh & wait )
( cd PolybenchC-Noarr-tbb && DATASET_SIZE=EXTRALARGE DATA_DIR=extralarge-data ./collect.sh & wait )
