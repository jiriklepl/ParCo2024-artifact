#!/bin/bash

set -eo pipefail

# This script compares the output of the C and C++/Noarr implementations of the Polybench benchmarks
# It assumes that the C++/Noarr implementations are built in the build directory and that the C implementations are built in the $POLYBENCH_C_DIR/build directory

export BUILD_DIR=${BUILD_DIR:-build}
export SKIP_DIFF=${SKIP_DIFF:-0}
export ALGORITHM=${ALGORITHM:-}

POLYBENCH_C_DIR="../PolybenchC-4.2.1"

dirname=$(mktemp -d)

cleanup() {
	echo "deleting $dirname" >&2
	rm -rf "$dirname"
}

trap cleanup EXIT

( cd "$POLYBENCH_C_DIR" && ./build.sh )
( cd . && ./build.sh )

find "$BUILD_DIR" -maxdepth 1 -executable -type f |
while read -r file; do
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

	echo "Comparing $filename"

	printf "\tNoarr:             "
	"$BUILD_DIR/$filename" 2>&1 1> "$dirname/cpp"

	if [ -f "$BUILD_DIR/${filename}d" ]; then
		printf "\tNoarr (autotuned): " >&2
		"$BUILD_DIR/${filename}d" 2>&1 1> "$dirname/cpp-autotuned"
	fi

	printf "\tBaseline:          "
	"$POLYBENCH_C_DIR/$BUILD_DIR/$filename" 2> "$dirname/c"

	if [ "$SKIP_DIFF" -eq 1 ]; then
		continue
	fi

	if [ -f "autotuned/$filename" ]; then
		paste <(grep -oE '[0-9]+\.[0-9]+' "$dirname/c") <(grep -oE '[0-9]+(\.[0-9]+)?' "$dirname/cpp") <(grep -oE '[0-9]+(\.[0-9]+)?' "$dirname/cpp-autotuned")
	else 
		paste <(grep -oE '[0-9]+\.[0-9]+' "$dirname/c") <(grep -oE '[0-9]+(\.[0-9]+)?' "$dirname/cpp")
	fi | awk "BEGIN {
		different = 0
		n = 0
		changes = 0
		autotune_changes = 0
		outputs = \"$([ -f "autotuned/$filename" ] && echo 3 || echo 2)\"
	}

	NF == outputs {
		n++
		if (\$1 != \$2 && changes < 10) {
			print \"baseline\", n, \$1
			print \"   noarr\", n, \$2
			changes++
			different = 1
		}
		if (outputs == 3 && \$1 != \$3 && autotune_changes < 10) {
			print \"baseline\", n, \$1
			print \"autotune\", n, \$3
			autotune_changes++
			different = 1
		}

		if (changes >= 10 || (outputs == 3 && autotune_changes >= 10))
			nextfile
		next
	}

	{ different = 1; nextfile }

	END {
		if (different) {
			printf \"Different output on %s \n\", \"$filename\"
			exit 1
		}
	}" 1>&2
done
