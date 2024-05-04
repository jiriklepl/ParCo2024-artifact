#!/bin/bash

export BUILD_DIR=${BUILD_DIR:-build}
export DATASET_SIZE=${DATASET_SIZE:-MEDIUM}
export DATA_TYPE=${DATA_TYPE:-FLOAT}
export NOARR_STRUCTURES_BRANCH=${NOARR_STRUCTURES_BRANCH:-ParCo2024}
export USE_SLURM=${USE_SLURM:-0}
export DATA_DIR=${DATA_DIR:-data}

# SLURM settings (if used)
export SLURM_ACCOUNT=${SLURM_ACCOUNT:-kdss}
export SLURM_PARTITION=${SLURM_PARTITION:-gpu-short}
export SLURM_WORKER=${SLURM_WORKER:-ampere01}
export SLURM_TIMEOUT=${SLURM_TIMEOUT:-"2:00:00"}

POLYBENCH_GPU_DIR="../PolyBenchGPU"

run_script() {
    if [ "$USE_SLURM" -eq 1 ]; then
        srun -A "$SLURM_ACCOUNT" -p "$SLURM_PARTITION" --exclusive -w"$SLURM_WORKER" --gres=gpu:1 -t "$SLURM_TIMEOUT" --mem=0 "$@"
    else
        "$@"
    fi
}

( cd "$POLYBENCH_GPU_DIR/CUDA" && run_script ./compileCodes.sh ) || exit 1
( cd . && run_script ./build.sh ) || exit 1

mkdir -p "$DATA_DIR"

compare_algorithms() {
    echo "collecting $1"
    ( run_script ./run_noarr_algorithm.sh "Noarr" "$1" & wait ) > "$DATA_DIR/$1.log"
	echo "" >> "$DATA_DIR/$1.log"
    ( run_script ./run_baseline_algorithm.sh "Baseline" "$2" & wait ) >> "$DATA_DIR/$1.log"
    echo "done"
}

compare_algorithms gemm "$POLYBENCH_GPU_DIR/CUDA/GEMM/gemm.exe"
compare_algorithms 2mm "$POLYBENCH_GPU_DIR/CUDA/2MM/2mm.exe"
compare_algorithms 2DConvolution "$POLYBENCH_GPU_DIR/CUDA/2DCONV/2DConvolution.exe"
compare_algorithms gramschmidt "$POLYBENCH_GPU_DIR/CUDA/GRAMSCHM/gramschmidt.exe"
compare_algorithms jacobi2d "$POLYBENCH_GPU_DIR/CUDA/JACOBI2D/jacobi2D.exe"
