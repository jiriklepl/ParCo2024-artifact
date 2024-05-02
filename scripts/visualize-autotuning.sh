#!/bin/bash -ex

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

echo "algorithm,implementation,time" > results/compare_compiles.csv
cat results/compare_compiles.log >> results/compare_compiles.csv

echo "record_type,algorithm,order,value" > results/autotuning.csv
./scripts/parse_autotune_log.awk results/autotuning.log >> results/autotuning.csv

./scripts/plot-compiles.R results/compare_compiles.csv results/autotuning.csv
