#!/bin/bash

prefix="$1"
file="$2"

printf "\t$prefix: "
"$file" 2>/dev/null

for _ in $(seq 10); do
    printf "\t$prefix: "
    "$file" 2>/dev/null
done
