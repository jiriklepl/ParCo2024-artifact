#!/bin/bash

./plot-tbb.R polybench-c-tbb.csv 1.3 3 TBB
./plot-others.R polybench-gpu.csv 1.3 3 GPU
./plot-polybench.R polybench-c.csv 4 3
./plot-others.R polybench-c-tuned.csv 1.3 3 serial
