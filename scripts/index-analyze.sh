#!/bin/bash

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <files...>"
    exit 1
fi

for file in "$@"; do
    if [ ! -f "$file" ]; then
        echo "File not found: $file"
        exit 1
    fi
done

# quite complicated so we avoid lambda expressions
INDEXATIONS=$(grep -oE '\w+(\[[^\[]+\])+' "$@" | wc -l)
LOOPS=$(grep -oE 'for\s*\([^)]+\)' "$@" | wc -l)

# again, quite complicated so we avoid lambda expressions
SUBSCRIPTS=$(grep -oE '\w+(\[[^\[]+\])+' "$@" | grep -oE '\[' | wc -l)
LOOP_SUBEXPRESSIONS=$(grep -oE 'for\s*\([^)]+\)' "$@" | grep -oE ';|\)' | wc -l)

printf "metric,indexation complexity,%d\n" $((INDEXATIONS + LOOPS))
printf "metric,subscript complexity,%d\n" $((SUBSCRIPTS + LOOP_SUBEXPRESSIONS))
