#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <file>"
    exit 1
fi

if [ ! -f "$1" ]; then
    echo "File not found: $1"
    exit 1
fi

# quite complicated so we avoid lambda expressions
INDEXATIONS=$(grep -oE '\w+(\[[^\[]+\])+' $1 | wc -l)
LOOPS=$(grep -oE 'for\s*\([^)]+\)' $1 | wc -l)

# again, quite complicated so we avoid lambda expressions
SUBSCRIPTS=$(grep -oE '\w+(\[[^\[]+\])+' $1 | grep -oE '\[' | wc -l)
LOOP_SUBEXPRESSIONS=$(grep -oE 'for\s*\([^)]+\)' $1 | grep -oE ';|\)' | wc -l)

printf "metric,indexation complexity,%d\n" $((INDEXATIONS + LOOPS))
printf "metric,subscript complexity,%d\n" $((SUBSCRIPTS + LOOP_SUBEXPRESSIONS))
