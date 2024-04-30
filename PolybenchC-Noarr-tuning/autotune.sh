#!/bin/bash -ex

export DATASET_SIZE=${DATASET_SIZE:-EXTRALARGE}
export DATA_TYPE=${DATA_TYPE:-FLOAT}
export NOARR_STRUCTURES_BRANCH=${NOARR_STRUCTURES_BRANCH:-main}
export NOARR_TUNING_BRANCH=${NOARR_TUNING_BRANCH:-main}
export NUM_RUNS=${NUM_RUNS:-25}
export USE_SLURM=${USE_SLURM:-0}
export VENV_DIR=${VENV_DIR:-env}
export BUILD_DIR=${BUILD_DIR:-build}

# SLURM settings (if used)
export SLURM_ACCOUNT=${SLURM_ACCOUNT:-kdss}
export SLURM_PARTITION=${SLURM_PARTITION:-mpi-homo-short}
export SLURM_WORKER=${SLURM_WORKER:-w201}
export SLURM_TIMEOUT=${SLURM_TIMEOUT:-"2:00:00"}

export ALGORITHM=${ALGORITHM:-}

run_script() {
    if [ "$USE_SLURM" -eq 1 ]; then
        srun -A "$SLURM_ACCOUNT" -p "$SLURM_PARTITION" --exclusive -w"$SLURM_WORKER" -t "$SLURM_TIMEOUT" --mem=0 "$@"
    else
        "$@"
    fi
}

( cd . && run_script ./build.sh ) >&2
( cd . && run_script ./prepare_env.sh ) >&2

mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"

for file in ./*_autotune; do
	filename=$(basename "$file")

	if [ -n "$ALGORITHM" ]; then
		case "$filename" in
			"$ALGORITHM"_autotune)
				;;
			*)
				continue
				;;
		esac
	fi

	echo "Autotuning $(sed -E 's/_autotune$//' <<< "$filename")"

	# run the program to generate the autotuning script
	run_script "./$filename" > "$filename-script.py"

	# run the autotuning script under the virtual environment
	if ! run_script ../env/bin/python3 "$filename-script.py" --test-limit="$NUM_RUNS" --no-dups --parallelism 1 2>&1 | tee "$filename-log.json" ||
		[ ! -f "mmm_final_config.json" ]
	then
		echo "Autotuning of $filename failed" >&2
		continue
	fi

	# transform the configuration into a list of defines
	config=$(sed -E 's/, "/; "/g' mmm_final_config.json | grep -oE '"\w+"\s*:\s*[^;}]+' | tr -d '":[]' | sed -E 's/, /,/g' | awk '{printf("%s", "-DNOARR_TUNING_PARAMETER_VALUE_" $1 "=" $2 " ")}END{print ""}')

	mv "mmm_final_config.json" "$filename.config.json"
	echo "$config" > "$filename.config.txt"

	# build the autotuned version of the program
	run_script cmake --build . -j"$(nproc)" -t "${filename}d" -- -B CXX_DEFINES="-D${DATASET_SIZE}_DATASET -DDATA_TYPE_IS_$DATA_TYPE ${CMAKE_CXX_FLAGS} $config"
done
