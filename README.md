# Artifact Submission: Pure C++ Approach to Optimized Parallel Traversal of Regular Data Structures

This is the replication package containing code and experimental results for the paper "Pure C++ Approach to Optimized Parallel Traversal of Regular Data Structures" submitted to the [PMAM 2024 workshop](https://www.cs.otago.ac.nz/pmam2024/) at the [PPoPP 2024 conference](https://conf.researchr.org/home/PPoPP-2024).

## Table of contents

- [Overview](#overview) - overview of the contents of the artifact
- [Requirements](#requirements) - software requirements for the experiments
- [Experiment reproduction](#experiment-reproduction) - steps for reproducing the experiments
- [Validation](#validation) - steps for validating the implementations
- [Code comparison](#code-comparison) - steps for comparing the code of the original Polybench/C benchmark and the Noarr implementation (summarization presented in the paper) and comparing the code changes required to perform the tuning transformations implemented in [PolybenchC-tuned](PolybenchC-tuned) on algorithms in [PolybenchC-pretune](PolybenchC-pretune) that are adjusted for the transformations

## Overview

The artifact contains experimental results on various modifications of the following benchmark suits:

- [Polybench/C-4.2.1](https://sourceforge.net/projects/polybench/files/)
- [PolyBench/GPU-1.0](https://github.com/sgrauerg/polybenchGpu)

The artifact uses the implementation of Noarr from [https://github.com/jiriklepl/noarr-structures](https://github.com/jiriklepl/noarr-structures), which is continuously tested on various platforms using GitHub Actions. It supports  the following compilers: GCC (10, 11, 12, 13), Clang (14, 15), NVCC (12), and MSVC (19)

The artifact structure:

- [running-examples](running-examples): Contains the examples of Noarr code presented in the paper.
- [plots](plots): Generated plots used in paper figures.
- [results](results): CSV files with the measured wall-clock times and the code comparison and archives with the measured wall-clock times; also contains the log file with the summarized statistics of the code comparison.
- relating to the Polybench/C-4.2.1 benchmark suite:
  - [PolybenchC-4.2.1](PolybenchC-4.2.1): The Polybench/C benchmark with custom build script for convenience and `flatten` pragmas for consistency with Noarr.
  - [PolybenchC-Noarr](PolybenchC-Noarr): Implementation of the Polybench/C benchmark using Noarr structures and Noarr Traversers.
  - [PolybenchC-pretune](PolybenchC-pretune): Modification of the algorithms from Polybench/C benchmark that are subjected to tuning in [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned). Some of the loops are broken down into multiple loops to allow better traversal options.
  - [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned): Tuned versions of four algorithms from [PolybenchC-Noarr](PolybenchC-Noarr) using Noarr Traversers. Each algorithm is taken from a different category of algorithms from the Polybench/C benchmark.
  - [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned): Implementation of the tuned algorithms from [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned) using Noarr Traversers.
  - [PolybenchC-tbb](PolybenchC-tbb): Parallelization of four algorithms from [PolybenchC-4.2.1](PolybenchC-4.2.1) using Intel TBB.
  - [PolybenchC-Noarr-tbb](PolybenchC-Noarr-tbb): Implementation of the parallelized algorithms from [PolybenchC-tbb](PolybenchC-tbb) using Noarr Traversers.
- relating to the PolyBench/GPU-1.0 benchmark suite:
  - [PolyBenchGPU](PolyBenchGPU): The PolyBench/GPU with minor bug fixes (mostly relating to wrong arguments) and modified dataset sizes for convenience of measurement.
  - [PolyBenchGPU-Noarr](PolyBenchGPU-Noarr): Implementation of the PolyBench/GPU benchmark using Noarr structures and Noarr Traversers.
- [transformations.md](transformations.md): Detailed list of transformations provided by Noarr Traversers.
- [scripts](scripts): Scripts used for running the experiments, validating the implementations, generating the plots presented in the paper and producing further data from the collected measurements.

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

The following software is required for the running examples presented in the paper:

- C++ compiler with support for C++20 - preferably GCC 12 or newer
- Intel TBB 2021.3 or newer
- NVCC 12 or newer
- CMake 3.10 or newer
- Python 3.6 or newer

## Experiment reproduction

The experiments can be reproduced using the following steps:

```bash
# clone the repository and enter it
git clone "https://github.com/jiriklepl/pmam2024-artifact.git"
cd pmam2024-artifact

# for the CPU experiments:
scripts/run-measurements-CPU.sh

# for the GPU experiments:
scripts/run-measurements-GPU.sh

# generate the plots presented in the paper:
scripts/generate_plots.sh

# -- optionally --

# generate additional visualizations of the measured wall-clock times and the code comparison:
scripts/more_visualizations.sh
```

In our laboratory cluster, we use the Slurm workload manager. Setting the `USE_SLURM` environment variable to `1` configures the scripts to use Slurm for running the experiments in the configuration that we used for the paper. The configuration can be modified in the scripts run by the [scripts/run-measurements-CPU.sh](scripts/run-measurements-CPU.sh) and [scripts/run-measurements-GPU.sh](scripts/run-measurements-GPU.sh) scripts.

After running the experiments, the plots presented in the paper can be generated using the [scripts/generate_plots.sh](scripts/generate_plots.sh) script. It runs the [scripts/parse_data.sh](scripts/parse_data.sh) script on the measured wall-clock times and then runs the R scripts in the root directory to generate the plots. The plots are stored in the [plots](plots) directory in the root directory.

This also generates the corresponding CSV files with the measured wall-clock times in the root directory.

### More visualizations

Additional visualizations of the measured wall-clock times and the code comparison can be generated by running the [scripts/more_visualizations.sh](scripts/more_visualizations.sh) script. The plots are stored in the [plots](plots) directory in the root directory.

## Validation

The validation can be performed using the following steps:

```bash
# clone the repository and enter it
git clone "https://github.com/jiriklepl/pmam2024-artifact.git"
cd pmam2024-artifact

# for the CPU experiments:
scripts/validate-CPU.sh

# for the GPU experiments:
scripts/validate-GPU.sh
```

This script runs the implementations of the Polybench/C and the tuned/TBB-parallelized versions as well as the PolyBench/GPU algorithms and compares their respective outputs with their Noarr counterparts. It outputs whether it found any mismatches in the outputs. Note that the validation scripts merely check whether the outputs of the implementations are the same (there is zero threshold for the difference). It provides a simple sanity check that the implementations are functionally equivalent (the baseline and the Noarr counterparts).

## Code comparison

```bash
# clone the repository and enter it
git clone "https://github.com/jiriklepl/pmam2024-artifact.git"
cd pmam2024-artifact

# for the Polybench/C benchmark:
scripts/code_compare.sh > "results/code_overall.log"

# for the tuning transformations:
mkdir -p results
scripts/compare_transformations.sh > "results/compare_transformations.log"
```

### Polybench/C benchmark and the Noarr implementation

These experiments are designed to provide some insights into where the Noarr approach is more verbose and where it saves some coding effort.

The code comparison can be performed by running the [scripts/code_compare.sh](scripts/code_compare.sh) script. It compares the code of the original Polybench/C benchmark and the Noarr implementation and outputs the differences into the file [PolybenchC-Noarr/statistics.csv](PolybenchC-Noarr/statistics.csv) and the summarized statistics to the standard output (as shown in [results/code_overall.log](results/code_overall.log)).

### Tuning transformations

The comparison of the transformations for tuning can be performed by running the [scripts/compare_transformations.sh](scripts/compare_transformations.sh) script. The output of the comparison is printed to the standard output (as shown in [results/compare_transformations.log](results/compare_transformations.log)).
