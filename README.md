# Artifact Submission: Pure C++ Approach to Optimized Parallel Traversal of Regular Data Structures

This is the replication package containing code and experimental results for the paper "Pure C++ Approach to Optimized Parallel Traversal of Regular Data Structures" submitted to the PMAM 2024 workshop.

## Table of contents

- [Overview](#overview) - overview of the contents of the artifact
- [Requirements](#requirements) - software requirements for the experiments
- [Experiment reproduction](#experiment-reproduction) - steps for reproducing the experiments
- [Validation](#validation) - steps for validating the implementations
- [Producing plots presented in the paper](#producing-plots-presented-in-the-paper) steps for generating the plots presented in the paper along with the corresponding CSV files from the measured wall-clock times
- [Code comparison](#code-comparison) - steps for comparing the code of the original Polybench/C benchmark and the Noarr implementation (summarization presented in the paper)
- [Comparing transformations for tuning](#comparing-transformations-for-tuning) - steps for comparing the code changes required to perform the tuning transformations implemented in [PolybenchC-tuned](PolybenchC-tuned) on algorithms in [PolybenchC-pretune](PolybenchC-pretune) that are adjusted for the transformations
- [More visualizations](#more-visualizations) - steps for generating additional visualizations of the measured wall-clock times and the code comparison along with the corresponding CSV files
- [Noarr Traverser-transformations](#noarr-traverser-transformations) - list of transformations provided by Noarr Traversers

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
  - [running-examples](running-examples): Contains the examples of Noarr code presented in the paper.

The artifact contains the following scripts:

- [run-measurements-CPU.sh](run-measurements-CPU.sh): Script for running the measurements on CPU.

  This script runs the measurements of Polybench/C and the tuned/TBB-parallelized versions in 10 repetitions with a warm-up run. The measured wall-clock times are stored in the `medium-data`, `large-data`, and `extralarge-data` directories in the respective benchmark directories (always in the ones ending with `-Noarr`). The measured wall-clock times are stored in `<algorithm>.log` files in the following format:

  ```log
  <implementation>: <wall-clock time>
  ```

  where `<implementation>` is either `Noarr` or `Baseline` and `<wall-clock time>` is the measured wall-clock time in seconds with 6 decimal places. The first line of each implementation is the warm-up run and it should be filtered out before analysis.

- [run-measurements-GPU.sh](run-measurements-GPU.sh): Script for running the measurements on GPU.

  This script runs the measurements of Polybench/GPU in 10 repetitions with a warm-up run. The measured wall-clock times are stored in the `data` directory in the `PolyBenchGPU-Noarr` directory. The log files are stored in the same format as in [run-measurements-CPU.sh](run-measurements-CPU.sh).

- [validate-CPU.sh](validate-CPU.sh): Script for validating the implementations of the algorithms for CPU.

  This script runs the implementations of the Polybench/C and the tuned/TBB-parallelized versions and compares their respective outputs with their Noarr counterparts. It outputs whether it found any mismatches in the outputs.

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

  On the standard output, it outputs the summarized statistics (as shown in `code_overall.log` in the `PolybenchC-Noarr` directory), comparing the number of lines, characters, and tokens of the original Polybench/C benchmark and the Noarr implementation after the same preprocessing. It also outputs the total size of the preprocessed SCOP regions compressed using gzip as single files and as a tar archive.

  it also outputs files `noarr.cpp` and `c.cpp` that contain the concatenated SCOP regions of the respective implementations for inspection.

- [compare_transformations.sh](compare_transformations.sh): Script for comparing the code changes required to perform the tuning transformations implemented in [PolybenchC-tuned](PolybenchC-tuned) on algorithms in [PolybenchC-pretune](PolybenchC-pretune) that are adjusted for the transformations. The changes are compared against Noarr proto-structures in [PolybenchC-Noarr-tuned](PolybenchC-Noarr-tuned) that perform the same transformations on the Noarr abstraction of the algorithms.

  On the standard output, it outputs the column-wise diff for the Polybench/C baseline and the list of proto-structures for the Noarr implementation for each algorithm preceded by the name of the algorithm. Each list is followed by the total number of changes or proto-structures.

- [more_visualizations.sh](more_visualizations.sh): Script for generating additional visualizations of the measured wall-clock times and the code comparison.

  This script generates additional visualizations of the measured wall-clock times and the code comparison that are referenced in the paper.

- [run-examples.sh](run-examples.sh): Script for running the examples of Noarr code presented in the paper.

  This script runs the examples of Noarr code presented in the paper on freshly generated input data and performs a simple validation of the outputs. The purpose of this script is to demonstrate that the presented Noarr code is functional and can be used in practice. The script requires Python 3.6 or newer on top of the requirements for the experiments.

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
# clone the repository
git clone "https://github.com/jiriklepl/pmam2024-artifact.git"

# for the CPU experiments:
./run-measurements-CPU.sh

# for the GPU experiments:
./run-measurements-GPU.sh
```

In our laboratory cluster, we use the Slurm workload manager. Setting the `USE_SLURM` environment variable to `1` configures the scripts to use Slurm for running the experiments in the configuration that we used for the paper. The configuration can be modified in the scripts run by the `run-measurements-CPU.sh` and `run-measurements-GPU.sh` scripts.

## Validation

The validation can be performed using the following steps:

```bash
# clone the repository
git clone "https://github.com/jiriklepl/pmam2024-artifact.git"

# for the CPU experiments:
./validate-CPU.sh

# for the GPU experiments:
./validate-GPU.sh
```

This script runs the implementations of the Polybench/C and the tuned/TBB-parallelized versions as well as the PolyBench/GPU algorithms and compares their respective outputs with their Noarr counterparts. It outputs whether it found any mismatches in the outputs. Note that the validation scripts merely check whether the outputs of the implementations are the same (there is zero threshold for the difference). It provides a simple sanity check that the implementations are functionally equivalent (the baseline and the Noarr counterparts).

## Producing plots presented in the paper

After running the experiments, the plots presented in the paper can be generated using the `generate_plots.sh` script. It runs the `parse_data.sh` script on the measured wall-clock times and then runs the R scripts in the root directory to generate the plots. The plots are stored in the `plots` directory in the root directory.

This also generates the corresponding CSV files with the measured wall-clock times in the root directory.

## Running examples

The running examples presented in the paper can be run using the `run-examples.sh` script that also generates their input data and performs a simple validation of the outputs. The script requires Python 3.6 or newer on top of the requirements for the experiments.

### Listings

Each code position beginning a code snippet presented in the paper as a listing is marked with a comment `// PAPER: <section.subsection number> - <capitalized ordinal> listing``

- Section 3.1

  - First, second and third listing: [running-examples/matmul.cpp](running-examples/matmul.cpp)
  - Fourth listing: [running-examples/matmul_factored.cpp](running-examples/matmul_factored.cpp)

- Section 4.0

  - First and second listing: [running-examples/histogram.cpp](running-examples/histogram.cpp)

- Section 4.1

  - First and second listing: [running-examples/histogram.cu](running-examples/histogram.cu)

- Section 4.2

  - First, second, third and fourth listing: [running-examples/histogram.cu](running-examples/histogram.cu)

## Code comparison

The code comparison can be performed by running the `PolybenchC-Noarr/code_compare.sh` script. It compares the code of the original Polybench/C benchmark and the Noarr implementation and outputs the differences into the file `statistics.csv` in the `PolybenchC-Noarr` directory and the summarized statistics to the standard output (as shown in `code_overall.log` in the `PolybenchC-Noarr` directory).

## Comparing transformations for tuning

The comparison of the transformations for tuning can be performed by running the `compare_transformations.sh` script. The output of the comparison is printed to the standard output (as shown in `compare_transformations.log`).

## More visualizations

Additional visualizations of the measured wall-clock times and the code comparison can be generated by running the `more_visualizations.sh` script. The plots are stored in the `plots` directory in the root directory.

This also generates the corresponding CSV files with the measured wall-clock times in the root directory, the `noarr.cpp` and `c.cpp` files that contain the concatenated SCOP regions of the `PolybenchC-Noarr` directory for inspection, the `statistics.csv` file in the `PolybenchC-Noarr` directory with the code comparison of the original Polybench/C benchmark and the Noarr implementation, and the `code_overall.log` file in the `PolybenchC-Noarr` directory with the summarized statistics of the code comparison that are referenced in the paper.

## Noarr Traverser transformations

The traversals performed by Noarr Traversers can be transformed by applying the `.order()` method with an argument that specifies the desired transformation, these transformations include: (the list is non-exhaustive; also, the Noarr library defines many shortcuts for various transformation combinations)

- `into_blocks<OldDim, NewMajorDim, NewMinorDim>(block_size)` - Separates the specified dimension into blocks according to the given block size; each index from the original index space is then uniquely associated with the cartesian product of a major index(block index) and a minor index. The dimension does not have to be contiguous. However, the transformation always assumes the dimension length is divisible by the block size; otherwise, some of the data are associated with indices outside the accessible index space and thus they cannot be traversed.

  For the cases where the original dimension length is not guaranteed to be divisible by the block size, the Noarr library defines:

  - `into_blocks_static<OldDim, NewIsBorderDim, NewMajorDim, NewMinorDim>(block_size)` - The `NewIsBorderDim` is the top-most of the three newly created dimensions, and the other two are dependent on its index. It has a length of `2`; and, if it is given the index `0`, the rest of `into_blocks_static` behaves just like `into_blocks`; if it is, instead, given the index `1`, then the length of the major dimension is set to `1` (there is only one block), and the length of the minor dimension is set to the division remainder (modulo) after dividing the original dimension length by the block size.

  - `into_blocks_dynamic<OldDim, NewMajorDim, NewMinorDim, NewDimIsPresent>(block_size)` - The specified dimension is divided into blocks according to the given block size using round-up division. This means that each index within the original index space is uniquely associated with some major index (block index) and some minor index and it is always accessible within the new index space (contrasting the simple `into_blocks`). Without the *is-present* dimension, this would allow for accessing items outside the original index space - for such items (we can recognize them by `oldLength <= majorIdx * block_size + minorIdx` being `true`), the is-present dimension length is `0`; otherwise, it is `1`. This behavior of the is-present dimension ensures that traversing over the legal items behaves as expected, while the traversal over the items outside the original index space traverses `0` items.

    This essentially emulates a pattern commonly found in algorithms for massively parallelized platforms (such as CUDA), where we map each thread to an element of an array (we assume an array for simplicity - but we can generalize) and then each thread starts its work by checking whether its corresponding index falls within the bounds of the array. For this case, we might use `into_blocks_dynamic<ArrayDim, GridDim, BlockDim, DimIsPresent>` and then use `auto ct = cuda_threads<GridDim, BlockDim>(transformed_traverser)` to bind the appropriate dimensions, the traversal performed by the `ct.inner()` inner traverser then transparently checks the array bound as if we used the `if (x >= array_length) return;` pattern manually.

- `merge_blocks<OldMajor, OldMinor, NewDim>()` - A transformation that performs the transformation inverse to `into_blocks`. It merges the specified major and minor dimensions into a single dimension, effectively applying loop fusion.

- `fix<Dim>(value)`, `fix<Dims...>(values...)`, `fix(state)` - Limits the traversal to a certain row, column, block, etc.

- `bcast<Dim>(length)` - Creates a placeholder dimension that consumes the corresponding input index. The resulting memory access is not affected by the choice of any particular index supplied to the dimension.

  - Typically used as a placeholder when binding dimensions via `cuda_threads`; or when adding blocking to some implementation configurations, we can use `bcast` in other configurations to preserve the same API.

- `rename<OldDim, NewDim>()` - Renames a dimension of a structure. This is especially useful when we want to unify or disconnect dimensions of two or more data structures figuring in a certain traversal. Or, we want to access the same data structure in two different ways.

- `shift<Dim>(offset)` - Makes the index provided for the specified dimension mapped to another one that is shifted by the given offset; also shortens the length of the traversal through the dimension accordingly so all shifted indices match the original range.

- `slice<Dim>(offset, length)` - This is a generalization of `shift` that also explicitly restricts the length of the specified dimension. This is useful whenever we require a traversal through a contiguous subset of indices.

  - `span<Dim>(offset_start, offset_end)` - A similar transformation to `slice` that specifies the end offset instead of the length.

- `hoist<Dim>()` - Moves the specified dimension to the outermost position in the abstracted loop nest. This is equivalent to a loop interchange.

Note that the listed transformations can be applied to memory layouts and traversals alike. The transformations are programmed in such a way they accept and propagate type-embedded literal arguments to enable further optimizations. It is also possible to provide more, user-defined, transformations as needed.
