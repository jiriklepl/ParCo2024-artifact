#!/usr/bin/env bash

export BUILD_DIR=${BUILD_DIR:-build}
export DATASET_SIZE=${DATASET_SIZE:-MEDIUM}
export DATA_TYPE=${DATA_TYPE:-FLOAT}
export NOARR_STRUCTURES_BRANCH=${NOARR_STRUCTURES_BRANCH:-main}
export USE_SLURM=${USE_SLURM:-0}
export DATA_DIR=${DATA_DIR:-data}

POLYBENCH_GPU_DIR="../PolyBenchGPU"

if [ "$USE_SLURM" -eq 1 ]; then
	RUN_SCRIPT="srun -A kdss -p gpu-short --exclusive -wampere01 --gres=gpu:1 -t 2:00:00"
else
	RUN_SCRIPT="$(which bash)"
fi

( cd "$POLYBENCH_GPU_DIR/CUDA" && $RUN_SCRIPT compileCodes.sh ) || exit 1
( cd . && $RUN_SCRIPT ./build.sh ) || exit 1


mkdir -p "$DATA_DIR"

compare_algorithms() {
    echo "collecting $1"
    ( $RUN_SCRIPT ./run_noarr_algorithm.sh "Noarr" "$1" & wait ) > "$DATA_DIR/$1.log"
	echo "" >> "$DATA_DIR/$1.log"
    ( $RUN_SCRIPT ./run_baseline_algorithm.sh "Baseline" "$2" & wait ) >> "$DATA_DIR/$1.log"
    echo "done"
}

compare_algorithms gemm "$POLYBENCH_GPU_DIR/CUDA/GEMM/gemm.exe"
compare_algorithms 2mm "$POLYBENCH_GPU_DIR/CUDA/2MM/2mm.exe"
compare_algorithms 2DConvolution "$POLYBENCH_GPU_DIR/CUDA/2DCONV/2DConvolution.exe"
compare_algorithms gramschmidt "$POLYBENCH_GPU_DIR/CUDA/GRAMSCHM/gramschmidt.exe"
compare_algorithms jacobi2d "$POLYBENCH_GPU_DIR/CUDA/JACOBI2D/jacobi2D.exe"
