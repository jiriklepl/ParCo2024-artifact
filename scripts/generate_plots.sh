#!/bin/bash -e

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

mkdir -p results

if [ -d results/PolybenchC-tbb ] && scripts/parse_data.sh results/PolybenchC-tbb/extralarge-data > results/polybench-c-tbb.csv; then
    echo Plotting polybench-c-tbb
    scripts/plot-para.R results/polybench-c-tbb.csv 1.3 2.5 TBB
fi

if [ -d results/PolyBenchGPU ] && scripts/parse_data.sh results/PolyBenchGPU/data > results/polybench-gpu.csv; then
    echo Plotting polybench-gpu
    scripts/plot-others.R results/polybench-gpu.csv 1.3 2.5 GPU
fi

if [ -d results/PolybenchC ] && scripts/parse_data.sh results/PolybenchC/extralarge-data > results/polybench-c.csv; then
    echo Plotting polybench-c
    scripts/plot-polybench.R results/polybench-c.csv 4 2.5
fi

if [ -d results/PolybenchC-tuned ] && scripts/parse_data.sh results/PolybenchC-tuned/extralarge-data > results/polybench-c-tuned.csv; then
    echo Plotting polybench-c-tuned
    scripts/plot-others.R results/polybench-c-tuned.csv 1.3 2.5 serial
fi

if [ -d results/PolybenchC-omp ] && scripts/parse_data.sh results/PolybenchC-omp/extralarge-data > results/polybench-c-omp.csv; then
    echo Plotting polybench-c-omp
    scripts/plot-para.R results/polybench-c-omp.csv 1.3 2.5 OpenMP
fi
