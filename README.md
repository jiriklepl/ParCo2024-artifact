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
  - [PolybenchGPU](PolybenchGPU): The Polybench/GPU with minor bug fixes (mostly relating to wrong arguments) and modified dataset sizes for convenience of measurement.
  - [PolybenchGPU-Noarr](PolybenchGPU-Noarr): Reimplementation of the Polybench/GPU benchmark using Noarr structures and Noarr Traversers.

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
