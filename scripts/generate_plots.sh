#!/bin/bash

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

mkdir -p results

scripts/parse_data.sh PolybenchC-Noarr/extralarge-data > results/polybench-c.csv
scripts/parse_data.sh PolybenchC-Noarr-tbb/extralarge-data > results/polybench-c-tbb.csv
scripts/parse_data.sh PolybenchC-Noarr-tuned/extralarge-data > results/polybench-c-tuned.csv
scripts/parse_data.sh PolyBenchGPU-Noarr/data > results/polybench-gpu.csv

scripts/plot-tbb.R results/polybench-c-tbb.csv 1.3 2.5 TBB
scripts/plot-others.R results/polybench-gpu.csv 1.3 2.5 GPU
scripts/plot-polybench.R results/polybench-c.csv 4 2.5
scripts/plot-others.R results/polybench-c-tuned.csv 1.3 2.5 serial
