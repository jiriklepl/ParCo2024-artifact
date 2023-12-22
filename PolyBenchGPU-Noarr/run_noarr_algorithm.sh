#!/bin/bash

prefix="$1"
algorithm="$2"

printf "\t$prefix: "
build/runner "$algorithm" 2>&1 1>/dev/null | grep -oE "[0-9]+\.[0-9]{2,}"

for _ in $(seq 10); do
    printf "\t$prefix: "
    build/runner "$algorithm" 2>&1 1>/dev/null | grep -oE "[0-9]+\.[0-9]{2,}"
done
