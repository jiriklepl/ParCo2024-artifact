#!/bin/bash -ex

BUILD_DIR=${BUILD_DIR:-build}
DATASET_SIZE=${DATASET_SIZE:-EXTRALARGE}
DATA_TYPE=${DATA_TYPE:-FLOAT}

# Create the build directory
cmake -E make_directory "$BUILD_DIR"
[ -f "$BUILD_DIR/.gitignore" ] || echo "*" > "$BUILD_DIR/.gitignore"
cd "$BUILD_DIR"

# Configure the build
cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -D${DATASET_SIZE}_DATASET -DDATA_TYPE_IS_$DATA_TYPE -D_POSIX_C_SOURCE=200809L " \
    -DCMAKE_CXX_FLAGS="-DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -D${DATASET_SIZE}_DATASET -DDATA_TYPE_IS_$DATA_TYPE -D_POSIX_C_SOURCE=200809L " \
    ..

# Build the project
NPROC=$(nproc)
cmake --build . --config Release -j"$NPROC"
