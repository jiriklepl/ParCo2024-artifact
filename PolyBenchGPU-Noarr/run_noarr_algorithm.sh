#!/bin/bash

prefix="$1"
algorithm="$2"

NUM_RUNS=${NUM_RUNS:-10}

printf "\t$prefix: "
build/runner "$algorithm" 2>&1 1>/dev/null | grep -oE "[0-9]+\.[0-9]{2,}"

for _ in $(seq "$NUM_RUNS"); do
    printf "\t$prefix: "
    build/runner "$algorithm" 2>&1 1>/dev/null | grep -oE "[0-9]+\.[0-9]{2,}"
done
