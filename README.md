# Artifact for Abstractions for C++ Code Optimizations in Parallel High-performance Applications

This is the replication package containing code and experimental results for the paper "Abstractions for C++ Code Optimizations in Parallel High-performance Applications" submitted to the special issue of the [Parallel Computing](https://www.sciencedirect.com/journal/parallel-computing/) journal for [The 15th International Workshop on Programming Models and Applications for Multicores and Manycores](https://www.cs.otago.ac.nz/pmam2024/).

The paper presents a continuation of the work on the Noarr library, which can be found at [https://github.com/ParaCoToUl/noarr-structures](https://github.com/ParaCoToUl/noarr-structures). The earlier work on this extension was presented at the PMAM 2024 workshop paper [Pure C++ Approach to Optimized Parallel Traversal of Regular Data Structures](https://dl.acm.org/doi/10.1145/3649169.3649247) (doi: [10.1145/3649169.3649247](https://doi.org/10.1145/3649169.3649247)). The work on the Noarr library was presented at the [ICA3PP 2022](https://ica3pp2022.compute.dtu.dk/) conference paper [Astute Approach to Handling Memory Layouts of Regular Data Structures](https://link.springer.com/chapter/10.1007/978-3-031-22677-9_27) (doi: [10.1007/978-3-031-22677-9_27](https://doi.org/10.1007/978-3-031-22677-9_27)).

The abstractions extending the Noarr library are implemented in the following two repositories:

- Noarr Traversers: [https://github.com/jiriklepl/noarr-structures](https://github.com/jiriklepl/noarr-structures)

  [![DOI](https://zenodo.org/badge/430242022.svg)](https://zenodo.org/doi/10.5281/zenodo.11112979)

- Noarr Tuning: [https://github.com/jiriklepl/noarr-tuning](https://github.com/jiriklepl/noarr-tuning)

  [![DOI](https://zenodo.org/badge/769593117.svg)](https://zenodo.org/doi/10.5281/zenodo.11112949)


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

The artifact uses the implementation of Noarr from [https://github.com/jiriklepl/noarr-structures](https://github.com/jiriklepl/noarr-structures), which is continuously tested on various platforms using GitHub Actions. The tests cover the following compilers: GCC (10, 11, 12, 13), Clang (14, 15), NVCC (12), and MSVC (19) on Linux, macOS, and Windows.

The artifact structure:

- [running-examples](running-examples): Contains the examples of Noarr code presented in the paper.
- [plots](plots): Generated plots used in paper figures.
- [results](results): CSV files with the measured wall-clock times and the code comparison and archives with the measured wall-clock times; also contains the log file with the summarized statistics of the code comparison and the autotuning results.
- relating to the Polybench/C-4.2.1 benchmark suite:
  - [PolybenchC-4.2.1](PolybenchC-4.2.1): The Polybench/C benchmark with custom build script for convenience and `flatten` pragmas for consistency with Noarr.
  - [PolybenchC-Noarr](PolybenchC-Noarr): Implementation of the Polybench/C benchmark using Noarr structures and Noarr Traversers.
  - [PolybenchC-pretune](PolybenchC-pretune): Modification of the algorithms from Polybench/C benchmark that are subjected to tuning in [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned). Some of the loops are broken down into multiple loops to allow better traversal options.
  - [PolybenchC-tuned](PolybenchC-tuned): Tuned versions of four algorithms from [PolybenchC-Noarr](PolybenchC-Noarr) ([PolybenchC-pretune](PolybenchC-pretune)). Each algorithm is taken from a different category of algorithms from the Polybench/C benchmark.
  - [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned): Implementation of the tuned algorithms from [PolybenchC-tuned](PolybenchC-tuned) using Noarr Traversers.
  - [PolybenchC-Noarr-tuning](PolybenchC-Noarr-tuning): Implementation of the autotuning of the Polybench/C benchmarks reimplemented using Noarr Structures and Noarr Traversers using Noarr Tuning.
  - [PolybenchC-tbb](PolybenchC-tbb): Parallelization of four algorithms from [PolybenchC-4.2.1](PolybenchC-4.2.1) using Intel TBB.
  - [PolybenchC-Noarr-tbb](PolybenchC-Noarr-tbb): Implementation of the parallelized algorithms from [PolybenchC-tbb](PolybenchC-tbb) using Noarr Traversers.
  - [PolyBenchC-omp](PolyBenchC-omp): Parallelization of four algorithms from [PolybenchC-4.2.1](PolybenchC-4.2.1) using OpenMP.
  - [PolybenchC-Noarr-omp](PolybenchC-Noarr-omp): Implementation of the parallelized algorithms from [PolybenchC-omp](PolybenchC-omp) using Noarr Traversers.
- relating to the PolyBench/GPU-1.0 benchmark suite:
  - [PolyBenchGPU](PolyBenchGPU): The PolyBench/GPU with minor bug fixes (mostly relating to wrong arguments) and modified dataset sizes for convenience of measurement.
  - [PolyBenchGPU-Noarr](PolyBenchGPU-Noarr): Implementation of the PolyBench/GPU benchmark using Noarr structures and Noarr Traversers.
- [transformations.md](transformations.md): Detailed list of transformations provided by Noarr Traversers.
- [scripts](scripts): Scripts used for running the experiments, validating the implementations, generating the plots presented in the paper, and producing further data from the collected measurements.


## Requirements

The artifact considers the following software requirements for the experiments:

- C++ compiler with support for C++20 - preferably GCC 12, or newer
- Intel TBB 2021.3 or newer
- NVCC 12 or newer
- CMake 3.10 or newer

The following software is required for the analysis of the results:

- AWK
- R with ggplot2 and dplyr packages
- gcc
- clang, clang-format
- standard tools: namely wc, gzip, tar
- Python 3.6 or newer

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
git clone "https://github.com/jiriklepl/ParCo2024-artifact.git"
cd ParCo2024-artifact

# for the CPU experiments:
scripts/run-measurements-CPU.sh

# for the GPU experiments:
scripts/run-measurements-GPU.sh

# generate the plots presented in the paper:
scripts/generate_plots.sh

# measure and visualize the compile time comparison and autotuning results:
scripts/autotuning-test.sh
scripts/visualize-autotuning.sh

# -- optionally --

# generate additional visualizations of the measured wall-clock times and the code comparison:
scripts/more_visualizations.sh
```

In our laboratory cluster, we use the [Slurm workload manager](https://slurm.schedmd.com/documentation.html). Setting the `USE_SLURM` environment variable to `1` configures the scripts to use Slurm for running the experiments in the configuration that we used for the paper.

This configuration can be modified in the scripts run by the [scripts/run-measurements-CPU.sh](scripts/run-measurements-CPU.sh), [scripts/run-measurements-GPU.sh](scripts/run-measurements-GPU.sh), and [scripts/autotuning-test.sh](scripts/autotuning-test.sh) scripts.

After running the experiments, the plots presented in the paper can be generated using the [scripts/generate_plots.sh](scripts/generate_plots.sh) script. It runs the [scripts/parse_data.sh](scripts/parse_data.sh) script on the measured wall-clock times and then runs the R scripts in the root directory to generate the plots. The plots are stored in the [plots](plots) directory in the root directory.

This also generates the corresponding CSV files with the measured wall-clock times in the root directory.


### More visualizations

Additional visualizations of the measured wall-clock times and the code comparison can be generated by running the [scripts/more_visualizations.sh](scripts/more_visualizations.sh) script. The plots are stored in the [plots](plots) directory in the root directory.


## Validation

The validation can be performed using the following steps:

```bash
# clone the repository and enter it
git clone "https://github.com/jiriklepl/ParCo2024-artifact.git"
cd ParCo2024-artifact

# for the CPU experiments:
scripts/validate-CPU.sh

# for more strict validation of the CPU experiments (runs ASan, UBSan, and leak sanitizer):
scripts/sanitize-CPU.sh

# for the GPU experiments:
scripts/validate-GPU.sh
```

This script runs the implementations of the Polybench/C and the tuned/TBB-parallelized/OpenMP-parallelized versions and the PolyBench/GPU baseline algorithms and compares their respective outputs with their Noarr counterparts. It reports any mismatches found in the outputs.

The validation scripts check whether the outputs of the implementations are the same (there is a zero threshold for the difference) on multiple datasets: SMALL, MEDIUM, LARGE, EXTRALARGE. The sanity check script runs the implementations on the SMALL dataset (and also compares the outputs).


## Code comparison

```bash
# clone the repository and enter it
git clone "https://github.com/jiriklepl/ParCo2024-artifact.git"
cd ParCo2024-artifact

mkdir -p results

# for the Polybench/C benchmark and the Noarr implementation:
scripts/code_compare.sh > "results/code_overall.log"

# for the tuning transformations related to PolybenchC-Noarr-tuned and PolybenchC-tuned:
scripts/compare_transformations.sh > "results/compare_transformations.log"
```


### Polybench/C benchmark and the Noarr implementation

These experiments are designed to provide some insights into where the Noarr approach is more verbose and where it saves some coding effort.

The code comparison can be performed by running the [scripts/code_compare.sh](scripts/code_compare.sh) script. It compares the code of the original Polybench/C benchmark and the Noarr implementation and outputs the differences into the file [PolybenchC-Noarr/statistics.csv](PolybenchC-Noarr/statistics.csv) and the summarized statistics to the standard output (as shown in [results/code_overall.log](results/code_overall.log)).


### Tuning transformations

The comparison of the transformations for tuning can be performed by running the [scripts/compare_transformations.sh](scripts/compare_transformations.sh) script. The output of the comparison is printed to the standard output (as shown in [results/compare_transformations.log](results/compare_transformations.log)).
