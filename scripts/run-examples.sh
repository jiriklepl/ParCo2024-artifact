#!/usr/bin/bash -ex

# Run the examples

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

cd running-examples

tempdir=$(mktemp -d)

trap "rm -rf $tempdir" EXIT

# Generate the input files
HISTOGRAM_SIZE=100M
MATRIX_SIZE=2048

# Generate the input files
./gen-text.sh "$tempdir/input1.txt" "$HISTOGRAM_SIZE"
./gen-matrices.py "$tempdir/input2.txt" "$MATRIX_SIZE"

# Build the project
./build.sh

# Run the examples
./build/histogram-cu "$tempdir/input1.txt" "$(du -bs "$tempdir/input1.txt" | awk '{print $1}')" > "$tempdir/output1.txt"
./build/histogram-cpp "$tempdir/input1.txt" "$(du -bs "$tempdir/input1.txt" | awk '{print $1}')" > "$tempdir/output2.txt"
./build/histogram-cpp-parallel "$tempdir/input1.txt" "$(du -bs "$tempdir/input1.txt" | awk '{print $1}')" > "$tempdir/output3.txt"

diff "$tempdir/output1.txt" "$tempdir/output2.txt"
diff "$tempdir/output1.txt" "$tempdir/output3.txt"

./build/matmul "$tempdir/input2.txt" "$MATRIX_SIZE" > "$tempdir/output4.txt"
./build/matmul-blocked "$tempdir/input2.txt" "$MATRIX_SIZE" > "$tempdir/output5.txt"
./build/matmul-factored "$tempdir/input2.txt" "$MATRIX_SIZE" > "$tempdir/output6.txt"

diff "$tempdir/output4.txt" "$tempdir/output5.txt"
diff "$tempdir/output4.txt" "$tempdir/output6.txt"
