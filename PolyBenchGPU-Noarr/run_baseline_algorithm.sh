#!/bin/bash

prefix="$1"
file="$2"

NUM_RUNS=${NUM_RUNS:-10}

printf "\t$prefix: "
"$file" 2>/dev/null | grep -oE "[0-9]+\.[0-9]{2,}"

for _ in $(seq "$NUM_RUNS"); do
    printf "\t$prefix: "
    "$file" 2>/dev/null | grep -oE "[0-9]+\.[0-9]{2,}"
done
