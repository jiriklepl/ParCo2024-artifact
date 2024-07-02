#!/bin/bash -ex

export DATASET_SIZE=${DATASET_SIZE:-EXTRALARGE}
export DATA_TYPE=${DATA_TYPE:-FLOAT}
export NOARR_STRUCTURES_BRANCH=${NOARR_STRUCTURES_BRANCH:-multimerge}
export NOARR_TUNING_BRANCH=${NOARR_TUNING_BRANCH:-ParCo2024}
export NUM_RUNS=${NUM_RUNS:-10}
export USE_SLURM=${USE_SLURM:-0}
export VENV_DIR=${VENV_DIR:-env}
export BUILD_DIR=${BUILD_DIR:-build}

# SLURM settings (if used)
export SLURM_ACCOUNT=${SLURM_ACCOUNT:-kdss}
export SLURM_PARTITION=${SLURM_PARTITION:-mpi-homo-short}
export SLURM_WORKER=${SLURM_WORKER:-w201}
export SLURM_TIMEOUT=${SLURM_TIMEOUT:-"2:00:00"}

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

cd "./PolybenchC-Noarr-tuning"

./autotune.sh 2>&1 | tee "../results/autotuning.log"
./compare_compiles.sh | tee "../results/compare_compiles.log"
