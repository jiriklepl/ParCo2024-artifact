#!/bin/bash

# This script will make the visualizations for the data in the `results/*/*data` folders

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

tmpdir=$(mktemp -d)
base=$(pwd -P)

trap "rm -rf $tmpdir" EXIT

cd "$tmpdir" || exit 1

mkdir -p results

"$base/scripts/parse_data.sh" "$base/results/PolybenchC/extralarge-data" > results/extralarge-polybench-c.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-tbb/extralarge-data" > results/extralarge-polybench-c-tbb.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-tuned/extralarge-data" > results/extralarge-polybench-c-tuned.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-omp/extralarge-data" > results/extralarge-polybench-c-omp.csv

"$base/scripts/parse_data.sh" "$base/results/PolybenchC/large-data" > results/large-polybench-c.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-tbb/large-data" > results/large-polybench-c-tbb.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-tuned/large-data" > results/large-polybench-c-tuned.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-omp/large-data" > results/large-polybench-c-omp.csv

"$base/scripts/parse_data.sh" "$base/results/PolybenchC/medium-data" > results/medium-polybench-c.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-tbb/medium-data" > results/medium-polybench-c-tbb.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-tuned/medium-data" > results/medium-polybench-c-tuned.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-omp/medium-data" > results/medium-polybench-c-omp.csv

"$base/scripts/parse_data.sh" "$base/results/PolybenchC/small-data" > results/small-polybench-c.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-tbb/small-data" > results/small-polybench-c-tbb.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-tuned/small-data" > results/small-polybench-c-tuned.csv
"$base/scripts/parse_data.sh" "$base/results/PolybenchC-omp/small-data" > results/small-polybench-c-omp.csv

( cd "$base" && scripts/code_compare.sh > "$tmpdir/results/code_overall.log" )

"$base/scripts/plot-tbb.R" results/extralarge-polybench-c-tbb.csv 1.3 2.5 TBB no
"$base/scripts/plot-polybench.R" results/extralarge-polybench-c.csv 4 2.5
"$base/scripts/plot-others.R" results/extralarge-polybench-c-tuned.csv 1.3 2.5 serial no
"$base/scripts/plot-others.R" results/extralarge-polybench-c-omp.csv 1.3 2.5 OpenMP no

"$base/scripts/plot-tbb.R" results/large-polybench-c-tbb.csv 1.3 2.5 TBB no
"$base/scripts/plot-polybench.R" results/large-polybench-c.csv 4 2.5
"$base/scripts/plot-others.R" results/large-polybench-c-tuned.csv 1.3 2.5 serial no
"$base/scripts/plot-others.R" results/large-polybench-c-omp.csv 1.3 2.5 OpenMP no

"$base/scripts/plot-tbb.R" results/medium-polybench-c-tbb.csv 1.3 2.5 TBB no
"$base/scripts/plot-polybench.R" results/medium-polybench-c.csv 4 2.5
"$base/scripts/plot-others.R" results/medium-polybench-c-tuned.csv 1.3 2.5 serial no
"$base/scripts/plot-others.R" results/medium-polybench-c-omp.csv 1.3 2.5 OpenMP no

"$base/scripts/plot-tbb.R" results/small-polybench-c-tbb.csv 1.3 2.5 TBB no
"$base/scripts/plot-polybench.R" results/small-polybench-c.csv 4 2.5
"$base/scripts/plot-others.R" results/small-polybench-c-tuned.csv 1.3 2.5 serial no
"$base/scripts/plot-others.R" results/small-polybench-c-omp.csv 1.3 2.5 OpenMP no

"$base/scripts/plot-statistics.R" "$base/results/statistics.csv"

mkdir -p "$base/results"

tar -czf "$base/results/visualizations.tar.gz" ./*

cd "$base" || exit 1

tar -xzf "results/visualizations.tar.gz"
