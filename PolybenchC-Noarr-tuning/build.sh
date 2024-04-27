#!/bin/bash -ex

export DATASET_SIZE=${DATASET_SIZE:-EXTRALARGE}
export DATA_TYPE=${DATA_TYPE:-FLOAT}
export NOARR_STRUCTURES_BRANCH=${NOARR_STRUCTURES_BRANCH:-main}
export NOARR_TUNING_BRANCH=${NOARR_TUNING_BRANCH:-main}
export BUILD_DIR=${BUILD_DIR:-build}

# Create the build directory
cmake -E make_directory "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure the build
cmake -DCMAKE_BUILD_TYPE=Release \
    -DNOARR_STRUCTURES_BRANCH="$NOARR_STRUCTURES_BRANCH" \
    -DNOARR_TUNING_BRANCH="$NOARR_TUNING_BRANCH" \
    -DCMAKE_CXX_FLAGS="-D${DATASET_SIZE}_DATASET -DDATA_TYPE_IS_$DATA_TYPE ${CMAKE_CXX_FLAGS}" \
    ..

# Build the project
NPROC=$(nproc)
cmake --build . --config Release -j"$NPROC" --target autotune
