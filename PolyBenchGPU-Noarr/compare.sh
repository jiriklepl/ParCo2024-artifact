#!/bin/bash

set -eo pipefail

export BUILD_DIR=${BUILD_DIR:-build}
export DATASET_SIZE=${DATASET_SIZE:-MEDIUM}
export DATA_TYPE=${DATA_TYPE:-FLOAT}
export NOARR_STRUCTURES_BRANCH=${NOARR_STRUCTURES_BRANCH:-main}
export SKIP_DIFF=${SKIP_DIFF:-0}
export ALGORITHM=${ALGORITHM:-}

POLYBENCH_GPU_DIR="../PolyBenchGPU"

dirname=$(mktemp -d)

cleanup() {
	echo "deleting $dirname" >&2
	rm -rf "$dirname"
}

trap cleanup EXIT

( cd "$POLYBENCH_GPU_DIR/CUDA" && bash compileCodes.sh )
( cd . && ./build.sh )

compare_algorithms() {
	filename=$(basename "$2")
	echo "Running $1" >&2

	if [ -n "$ALGORITHM" ]; then
		case "$filename" in
			"$ALGORITHM")
				;;
			*)
				continue
				;;
		esac
	fi

	printf "\tnoarr: " >&2
    "$BUILD_DIR/runner" "$1" 2>&1 >"$dirname/noarr.log" | grep -oE "[0-9]+\.[0-9]{2,}" >&2

	printf "\tbaseline: " >&2
    "$2" 2>"$dirname/baseline.log" | grep -oE "[0-9]+\.[0-9]{2,}" >&2

	if [ "$SKIP_DIFF" -eq 1 ]; then
		return
	fi

	paste <(grep -oE '[0-9]+\.[0-9]+|nan' "$dirname/baseline.log") <(grep -oE '[0-9]+(\.[0-9]+)?|nan' "$dirname/noarr.log") |
	awk "BEGIN {
		different = 0
		n = 0
		changes = 0
	}

	{
		n++
		if (\$1 != \$2 && changes < 10) {
			print \"baseline\", n, \$1
			print \"   noarr\", n, \$2
			changes++
			different = 1
		}

		if (changes >= 10)
			nextfile
		next
	}

	{ different = 1; nextfile }

	END {
		if (different) {
			printf \"Different output on %s \n\", \"$1\"
			exit 1
		}
	}" 1>&2
}

compare_algorithms gemm "$POLYBENCH_GPU_DIR/CUDA/GEMM/gemm.exe"
compare_algorithms 2mm "$POLYBENCH_GPU_DIR/CUDA/2MM/2mm.exe"
compare_algorithms 2DConvolution "$POLYBENCH_GPU_DIR/CUDA/2DCONV/2DConvolution.exe"
compare_algorithms gramschmidt "$POLYBENCH_GPU_DIR/CUDA/GRAMSCHM/gramschmidt.exe"
compare_algorithms jacobi2d "$POLYBENCH_GPU_DIR/CUDA/JACOBI2D/jacobi2D.exe"
