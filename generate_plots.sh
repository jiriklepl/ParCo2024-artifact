#!/bin/bash

./parse_data.sh PolybenchC-Noarr/extralarge-data > polybench-c.csv
./parse_data.sh PolybenchC-Noarr-tbb/extralarge-data > polybench-c-tbb.csv
./parse_data.sh PolybenchC-Noarr-tuned/extralarge-data > polybench-c-tuned.csv
./parse_data.sh PolyBenchGPU-Noarr/data > polybench-gpu.csv

./plot-tbb.R polybench-c-tbb.csv 1.3 2.5 TBB
./plot-others.R polybench-gpu.csv 1.3 2.5 GPU
./plot-polybench.R polybench-c.csv 4 2.5
./plot-others.R polybench-c-tuned.csv 1.3 2.5 serial
