#!/bin/bash -e

# Set the branch of Noarr Structures to use
export DATASET_SIZE=${DATASET_SIZE:-MEDIUM}
export DATA_TYPE=${DATA_TYPE:-FLOAT}
export NOARR_STRUCTURES_BRANCH=${NOARR_STRUCTURES_BRANCH:-ParCo2024}

export PATH=$PATH:/usr/local/cuda/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/cuda/lib:/usr/local/cuda/lib64

# Create the build directory
cmake -E make_directory build
cd build

# Configure the build
cmake -DCMAKE_BUILD_TYPE=Release \
    -DNOARR_STRUCTURES_BRANCH="$NOARR_STRUCTURES_BRANCH" \
	-DCMAKE_CUDA_FLAGS="${CMAKE_CUDA_FLAGS} -D${DATASET_SIZE}_DATASET -DDATA_TYPE_IS_$DATA_TYPE -O3" \
	..

# Build the project
NPROC=$(nproc)
cmake --build . --config Release -j"$NPROC"
