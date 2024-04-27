#!/bin/bash -ex

BUILD_DIR=${BUILD_DIR:-build}
NOARR_STRUCTURES_BRANCH=${NOARR_STRUCTURES_BRANCH:-main}

export PATH=$PATH:/usr/local/cuda/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/cuda/lib:/usr/local/cuda/lib64

# Create the build directory
cmake -E make_directory "$BUILD_DIR"
[ -f "$BUILD_DIR/.gitignore" ] || echo "*" > "$BUILD_DIR/.gitignore"
cd "$BUILD_DIR"

# Configure the build
cmake -DCMAKE_BUILD_TYPE=Release .. -DNOARR_STRUCTURES_BRANCH="$NOARR_STRUCTURES_BRANCH"

# Build the project
NPROC=$(nproc)
cmake --build . --config Release -j"$NPROC"
