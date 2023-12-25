#!/usr/bin/bash

export BUILD_DIR=${BUILD_DIR:-build}
export DATASET_SIZE=${DATASET_SIZE:-EXTRALARGE}
export DATA_TYPE=${DATA_TYPE:-FLOAT}
export NOARR_STRUCTURES_BRANCH=${NOARR_STRUCTURES_BRANCH:-main}
export USE_SLURM=${USE_SLURM:-0}
export DATA_DIR=${DATA_DIR:-data}

POLYBENCH_C_DIR="../PolybenchC-4.2.1"

if [ "$USE_SLURM" -eq 1 ]; then
	RUN_SCRIPT="srun -A kdss -p mpi-homo-short --exclusive -ww201 -t 2:00:00"
else
	RUN_SCRIPT="$(which bash)"
fi

( cd "$POLYBENCH_C_DIR" && $RUN_SCRIPT ./build.sh ) || exit 1
( cd . && $RUN_SCRIPT ./build.sh ) || exit 1


mkdir -p "$DATA_DIR"

find "$BUILD_DIR" -maxdepth 1 -executable -type f |
while read -r file; do
    filename=$(basename "$file")

    echo "collecting $filename"
    ( $RUN_SCRIPT ./run_noarr_algorithm.sh "Noarr" "$BUILD_DIR/$filename" & wait ) > "$DATA_DIR/$filename.log"
	echo "" >> "$DATA_DIR/$filename.log"
    ( $RUN_SCRIPT ./run_c_algorithm.sh "Baseline" "$POLYBENCH_C_DIR/$BUILD_DIR/$filename" & wait ) >> "$DATA_DIR/$filename.log"
    echo "done"
done
