#!/bin/bash

set -eo pipefail

# This script compares the output of the C and C++/Noarr implementations of the Polybench benchmarks
# It assumes that the C++/Noarr implementations are built in the build directory and that the C implementations are built in the $POLYBENCH_C_DIR/build directory

export BUILD_DIR=${BUILD_DIR:-build}

export DATASET_SIZE=${DATASET_SIZE:-MEDIUM}
export DATA_TYPE=${DATA_TYPE:-FLOAT}
export NOARR_STRUCTURES_BRANCH=${NOARR_STRUCTURES_BRANCH:-multimerge}
export NOARR_TUNING_BRANCH=${NOARR_TUNING_BRANCH:-ParCo2024}
export NUM_RUNS=${NUM_RUNS:-1}
export USE_SLURM=${USE_SLURM:-0}

# SLURM settings (if used)
export SLURM_ACCOUNT=${SLURM_ACCOUNT:-kdss}
export SLURM_PARTITION=${SLURM_PARTITION:-mpi-homo-short}
export SLURM_WORKER=${SLURM_WORKER:-w201}
export SLURM_TIMEOUT=${SLURM_TIMEOUT:-"2:00:00"}

export ALGORITHM=${ALGORITHM:-}

tmpdir=$(mktemp -d)

cleanup() {
	echo "deleting $tmpdir" >&2
	rm -rf "$tmpdir"
}

trap cleanup EXIT

POLYBENCH_C_DIR="../PolybenchC-4.2.1"
POLYBENCH_NOARR_DIR="../PolybenchC-Noarr"

run_script() {
    if [ "$USE_SLURM" -eq 1 ]; then
        srun -A "$SLURM_ACCOUNT" -p "$SLURM_PARTITION" --exclusive -w"$SLURM_WORKER" -t "$SLURM_TIMEOUT" --mem=0 \
			sh -c '"$@" 2>&1 >/dev/null' sh "$@"
    else
        "$@" 2>&1 >/dev/null
    fi
}

( cd "$POLYBENCH_C_DIR" && run_script ./build.sh ) >&2
( cd "$POLYBENCH_NOARR_DIR" && run_script ./build.sh ) >&2
( cd . && run_script ./build.sh ) >&2

exec 3< <(find "$BUILD_DIR" -maxdepth 1 -executable -type f)
while read -u 3 -r file; do
	filename=$(basename "$file")

	case "$filename" in
		*_autotune*)
			continue
			;;
	esac

	if [ -n "$ALGORITHM" ]; then
		case "$filename" in
			"$ALGORITHM")
				;;
			*)
				continue
				;;
		esac
	fi

	(
		cd "$POLYBENCH_NOARR_DIR/$BUILD_DIR" || exit 1

		run_script env time -f "%e" cmake --build . --target "$filename" -- -B &>/dev/null

		for _ in $(seq 1 "$NUM_RUNS"); do
			printf "%s" "$filename,noarr,"
			run_script env time -f "%e" cmake --build . --target "$filename" -- -B
		done
	)

	if [ -f "$BUILD_DIR/${filename}_autotune.config.txt" ]; then
		(
			cd "$BUILD_DIR" || exit 1
			config=$(cat "${filename}_autotune.config.txt")

			run_script env time -f "%e" cmake --build . --target "${filename}_autotuned" -- -B CXX_DEFINES="-D${DATASET_SIZE}_DATASET -DDATA_TYPE_IS_$DATA_TYPE ${CMAKE_CXX_FLAGS} $config" &>/dev/null

			for _ in $(seq 1 "$NUM_RUNS"); do
				printf "%s" "$filename,noarr-autotuned,"
				run_script env time -f "%e" cmake --build . --target "${filename}_autotuned" -- -B CXX_DEFINES="-D${DATASET_SIZE}_DATASET -DDATA_TYPE_IS_$DATA_TYPE ${CMAKE_CXX_FLAGS} $config"
			done
		)
	fi

	(
		cd "$POLYBENCH_C_DIR/$BUILD_DIR" || exit 1

		run_script env time -f "%e" cmake --build . --target "$filename" -- -B &>/dev/null

		for _ in $(seq 1 "$NUM_RUNS"); do
			printf "%s" "$filename,baseline,"
			run_script env time -f "%e" cmake --build . --target "$filename" -- -B
		done
	)
done
exec 3<&-
