#!/usr/bin/bash

# This script will make the visualizations for the data in the `*/*data` folders

tmpdir=$(mktemp -d)
base=$(pwd -P)

trap "rm -rf $tmpdir" EXIT

cd "$tmpdir" || exit 1

"$base/parse_data.sh" "$base/PolybenchC-Noarr/extralarge-data" > extralarge-polybench-c.csv
"$base/parse_data.sh" "$base/PolybenchC-Noarr-tbb/extralarge-data" > extralarge-polybench-c-tbb.csv
"$base/parse_data.sh" "$base/PolybenchC-Noarr-tuned/extralarge-data" > extralarge-polybench-c-tuned.csv

"$base/parse_data.sh" "$base/PolybenchC-Noarr/large-data" > large-polybench-c.csv
"$base/parse_data.sh" "$base/PolybenchC-Noarr-tbb/large-data" > large-polybench-c-tbb.csv
"$base/parse_data.sh" "$base/PolybenchC-Noarr-tuned/large-data" > large-polybench-c-tuned.csv

"$base/parse_data.sh" "$base/PolybenchC-Noarr/medium-data" > medium-polybench-c.csv
"$base/parse_data.sh" "$base/PolybenchC-Noarr-tbb/medium-data" > medium-polybench-c-tbb.csv
"$base/parse_data.sh" "$base/PolybenchC-Noarr-tuned/medium-data" > medium-polybench-c-tuned.csv

"$base/parse_data.sh" "$base/PolyBenchGPU-Noarr/data" > polybench-gpu.csv

( cd "$base/PolybenchC-Noarr" && ./code_compare.sh > code_overall.log )

"$base/plot-tbb.R" extralarge-polybench-c-tbb.csv 1.3 2.5 TBB no
"$base/plot-polybench.R" extralarge-polybench-c.csv 4 2.5
"$base/plot-others.R" extralarge-polybench-c-tuned.csv 1.3 2.5 serial no

"$base/plot-tbb.R" large-polybench-c-tbb.csv 1.3 2.5 TBB no
"$base/plot-polybench.R" large-polybench-c.csv 4 2.5
"$base/plot-others.R" large-polybench-c-tuned.csv 1.3 2.5 serial no

"$base/plot-tbb.R" medium-polybench-c-tbb.csv 1.3 2.5 TBB no
"$base/plot-polybench.R" medium-polybench-c.csv 4 2.5
"$base/plot-others.R" medium-polybench-c-tuned.csv 1.3 2.5 serial no

"$base/plot-others.R" polybench-gpu.csv 1.3 2.5 GPU no

"$base/plot-statistics.R" "$base/PolybenchC-Noarr/statistics.csv"

tar -czf "$base/visualizations.tar.gz" ./*
