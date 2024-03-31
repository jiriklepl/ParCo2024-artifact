#!/bin/bash

# This script will make the visualizations for the data in the `*/*data` folders

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

tmpdir=$(mktemp -d)
base=$(pwd -P)

trap "rm -rf $tmpdir" EXIT

cd "$tmpdir" || exit 1

mkdir -p results

"$base/scripts/parse_data.sh" "$base/PolybenchC-Noarr/extralarge-data" > results/extralarge-polybench-c.csv
"$base/scripts/parse_data.sh" "$base/PolybenchC-Noarr-tbb/extralarge-data" > results/extralarge-polybench-c-tbb.csv
"$base/scripts/parse_data.sh" "$base/PolybenchC-Noarr-tuned/extralarge-data" > results/extralarge-polybench-c-tuned.csv

"$base/scripts/parse_data.sh" "$base/PolybenchC-Noarr/large-data" > results/large-polybench-c.csv
"$base/scripts/parse_data.sh" "$base/PolybenchC-Noarr-tbb/large-data" > results/large-polybench-c-tbb.csv
"$base/scripts/parse_data.sh" "$base/PolybenchC-Noarr-tuned/large-data" > results/large-polybench-c-tuned.csv

"$base/scripts/parse_data.sh" "$base/PolybenchC-Noarr/medium-data" > results/medium-polybench-c.csv
"$base/scripts/parse_data.sh" "$base/PolybenchC-Noarr-tbb/medium-data" > results/medium-polybench-c-tbb.csv
"$base/scripts/parse_data.sh" "$base/PolybenchC-Noarr-tuned/medium-data" > results/medium-polybench-c-tuned.csv

( cd "$base" && scripts/code_compare.sh > "$tmpdir/results/code_overall.log" )

"$base/scripts/plot-tbb.R" results/extralarge-polybench-c-tbb.csv 1.3 2.5 TBB no
"$base/scripts/plot-polybench.R" results/extralarge-polybench-c.csv 4 2.5
"$base/scripts/plot-others.R" results/extralarge-polybench-c-tuned.csv 1.3 2.5 serial no

"$base/scripts/plot-tbb.R" results/large-polybench-c-tbb.csv 1.3 2.5 TBB no
"$base/scripts/plot-polybench.R" results/large-polybench-c.csv 4 2.5
"$base/scripts/plot-others.R" results/large-polybench-c-tuned.csv 1.3 2.5 serial no

"$base/scripts/plot-tbb.R" results/medium-polybench-c-tbb.csv 1.3 2.5 TBB no
"$base/scripts/plot-polybench.R" results/medium-polybench-c.csv 4 2.5
"$base/scripts/plot-others.R" results/medium-polybench-c-tuned.csv 1.3 2.5 serial no

"$base/scripts/plot-statistics.R" "$base/PolybenchC-Noarr/statistics.csv"

mkdir -p "$base/results"

tar -czf "$base/results/visualizations.tar.gz" ./*

cd "$base" || exit 1

tar -xzf "results/visualizations.tar.gz"
