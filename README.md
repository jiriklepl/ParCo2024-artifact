# Artifact Submission: Pure C++ Approach to Optimized Parallel Traversal of Regular Data Structures

This is the replication package containing code and experimental results for the paper "Pure C++ Approach to Optimized Parallel Traversal of Regular Data Structures" submitted to the PMAM 2024 workshop.

## Overview

The artifact contains experimental results on various modifications of the following benchmark suits:

- Polybench/C-4.2.1 - The original Polybench/C benchmark suite can be found at [https://sourceforge.net/projects/polybench/files/](https://sourceforge.net/projects/polybench/files/).
- PolyBench/GPU-1.0 - The original PolyBench/GPU benchmark suite can be found at [https://github.com/sgrauerg/polybenchGpu](https://github.com/sgrauerg/polybenchGpu).

The artifact uses the implementation of Noarr from [https://github.com/jiriklepl/noarr-structures](https://github.com/jiriklepl/noarr-structures), which is continuously tested on various platforms using GitHub Actions. It supports  the following compilers: GCC (10, 11, 12, 13), Clang (14, 15), NVCC (12), and MSVC (19)

The artifact contains the following directories:

- relating to the Polybench/C-4.2.1 benchmark suite:
  - [PolybenchC-4.2.1](PolybenchC-4.2.1): The Polybench/C benchmark with custom build script for convenience and `flatten` pragmas for consistency with Noarr.
  - [PolybenchC-Noarr](PolybenchC-Noarr): Reimplementation of the Polybench/C benchmark using Noarr structures and Noarr Traversers.
  - [PolybenchC-pretune](PolybenchC-pretune): Modification of the algorithms from Polybench/C benchmark that are subjected to tuning in [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned). Some of the loops are broken down into multiple loops to allow better traversal options.
  - [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned): Tuned versions of four algorithms from [PolybenchC-Noarr](PolybenchC-Noarr) using Noarr Traversers. Each algorithm is taken from a different category of algorithms from the Polybench/C benchmark.
  - [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned): Reimplementation of the tuned algorithms from [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned) using Noarr Traversers.
  - [PolybenchC-tbb](PolybenchC-tbb): Parallelization of four algorithms from [PolybenchC-4.2.1](PolybenchC-4.2.1) using Intel TBB.
  - [PolybenchC-Noarr-tbb](PolybenchC-Noarr-tbb): Reimplementation of the parallelized algorithms from [PolybenchC-tbb](PolybenchC-tbb) using Noarr Traversers.
- relating to the PolyBench/GPU-1.0 benchmark suite:
  - [PolyBenchGPU](PolyBenchGPU): The PolyBench/GPU with minor bug fixes (mostly relating to wrong arguments) and modified dataset sizes for convenience of measurement.
  - [PolyBenchGPU-Noarr](PolyBenchGPU-Noarr): Reimplementation of the PolyBench/GPU benchmark using Noarr structures and Noarr Traversers.

The artifact contains the following scripts:

- [run-measurements-CPU.sh](run-measurements-CPU.sh): Script for running the measurements on CPU.

  This script runs the measurements of Polybench/C and the tuned/tbb-paralleized versions in 10 repetitions with a warm-up run. The measured wall-clock times are stored in the `medium-data`, `large-data`, and `extralarge-data` directories in the respective benchmark directories (always in the ones ending with `-Noarr`). The measured wall-clock times are stored in `<algorithm>.log` files in the following format:

  ```log
  <implementation>: <wall-clock time>
  ```

  where `<implementation>` is either `Noarr` or `Baseline` and `<wall-clock time>` is the measured wall-clock time in seconds with 6 decimal places. The first line of each implementation is the warm-up run and it should be filtered out before analysis.

- [run-measurements-GPU.sh](run-measurements-GPU.sh): Script for running the measurements on GPU.

  This script runs the measurements of Polybench/GPU in 10 repetitions with a warm-up run. The measured wall-clock times are stored in the `data` directory in the `PolyBenchGPU-Noarr` directory. The log files are stored in the same format as in [run-measurements-CPU.sh](run-measurements-CPU.sh).

- [validate-CPU.sh](validate-CPU.sh): Script for validating the implementations of the algorithms for CPU.

  This script runs the implementations of the Polybench/C and the tuned/tbb-paralleized versions and compares their respective outputs with their Noarr counterparts. It outputs whether it found any mismatches in the outputs.

- [validate-GPU.sh](validate-GPU.sh): Script for validating the implementation of the algorithms for GPU.

  This script runs the implementations of the Polybench/GPU and compares their respective outputs with their Noarr counterparts. It outputs whether it found any mismatches in the outputs.

- [parse_data.sh](parse_data.sh): Script for parsing the measured wall-clock times.

  This script parses the measured wall-clock times from the given `data` directory (containing the log files in the format described above), filters out the warm-up runs, and outputs the results in the following comma-separated format (CSV) to the standard output:

  ```csv
  <name>,<implementation>,<wall-clock time>
  ```

  where `<name>` is the name of the algorithm, `<implementation>` is either `noarr` or `baseline`, and `<wall-clock time>` is the measured wall-clock time in seconds with 6 decimal places.

- [generate_plots.sh](generate_plots.sh): Script for generating the plots from the measured wall-clock times using `parse_data.sh` and R scripts in the root directory.

  This script generates the plots from the measured wall-clock times that are presented in the paper.

- [PolybenchC-Noarr/code_compare.sh](PolybenchC-Noarr/code_compare.sh): Script for comparing the code of the original Polybench/C benchmark and the Noarr implementation.

  This script compares the code of the original Polybench/C benchmark and the Noarr implementation and outputs the differences in the following comma-separated format (CSV) into the file `statistics.csv` in the `PolybenchC-Noarr` directory:

  ```csv
  <implementation>,<algorithm>,<lines>,<characters>,<tokens>,<gzip-size>
  ```
  
  where `<implementation>` is either `noarr` or `baseline`, `<algorithm>` is the name of the algorithm, `<lines>` is the number of lines enclosed within SCOP regions after stripping any comments and applying clang-format, `<characters>` is then the number of characters, `<tokens>` is the number of single C/C++ tokens, and `<gzip-size>` is the preprocessed SCOP region compressed using gzip.

  it also outputs files noarr.cpp and c.cpp that contain the concatenated SCOP regions of the respective implementations for inspection.

## Requirements

The artifact considers the following software requirements for the experiments:

- C++ compiler with support for C++20 - preferably GCC 12 or newer
- Intel TBB 2021.3 or newer
- NVCC 12 or newer
- CMake 3.10 or newer

The following software is required for the analysis of the results:

- AWK
- R with ggplot2 and dplyr packages
- gcc
- clang, clang-format
- standard tools: namely wc, gzip, tar

## Experiment reproduction

TODO

## Analysis of the results

TODO
