#!/bin/bash

tmpdir=$(mktemp -d) || exit 1

trap "rm -rf $tmpdir" EXIT

git clone --branch=pretune "https://github.com/jiriklepl/PolyBenchC-4.2.1.git" "$tmpdir/polybench-pretune" || exit 1
git clone --branch=tuned "https://github.com/jiriklepl/PolyBenchC-4.2.1.git" "$tmpdir/polybench-tuned" || exit 1
git clone --branch=tuned "https://github.com/jiriklepl/PMAM2024-artifact" "$tmpdir/polybench-noarr" || exit 1

measure_c_changes() {
    # cloc --diff --csv --include-lang=C "$tmpdir/polybench-pretune" "$tmpdir/polybench-tuned" |
    #     awk -F", " 'NR >= 2{ print $15 + $16 + $17 }' | # sum up the changes in C lines
    #     awk 'BEGIN{ sum = 0 }{ sum += $1 }END{ print sum }' # sum up the changes in all files

    CHANGES=$(find "$tmpdir/polybench-tuned" -type f -name "*.c" | while read -r file; do
        pretune="$tmpdir/polybench-pretune/${file#"$tmpdir"/polybench-tuned/}"

        if ! diff -y --suppress-common-lines "$pretune" "$file"; then
            echo "" >> "$tmpdir/diff"
        fi |
            grep -vE '^diff|^Only in' |
            grep -vE '^\s*int' | tee --append "$tmpdir/diff"
    done | wc -l)

    cat "$tmpdir/diff"

    echo "Changes in Polybench/C: $CHANGES"
}

measure_noarr() {
    find "$tmpdir/polybench-noarr" -type f -name "*.cpp" | while read -r file; do
        case "$file" in
            */datamining/*|*/linear-algebra/*|*/medley/*|*/stencils/*)
                ;;
            *)
                continue
                ;;
        esac

        case "$file" in
            */2mm.cpp|*/gemm.cpp|*/floyd-warshall.cpp|*/covariance.cpp)
                ;;
            *)
                continue
                ;;
        esac

        echo "$file"
    done |
    xargs awk '
BEGIN{
    last_filename = ""
    proto_structs = 0
}
/DEFINE_PROTO_STRUCT/ && # count proto structs (we use this macro so they are easy to find)
!/_layout/ && !/neutral_proto/{ # ignore layouts and neutral proto (they are there only for demonstration of generality)
    print
    proto_structs++
}
FILENAME != last_filename {
    if (last_filename != "") print ""
    last_filename = FILENAME
}
END{
    print "Changes in Noarr: " proto_structs
}
'
}

measure_c_changes
measure_noarr
