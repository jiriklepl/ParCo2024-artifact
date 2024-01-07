#!/bin/bash

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

tmpdir=$(mktemp -d) || exit 1

trap "rm -rf $tmpdir" EXIT

measure_c_changes() {
    CHANGES=$(find "PolybenchC-tuned" -mindepth 3 -maxdepth 4 -type f -name "*.c" | sort | while read -r file; do
        pretune="PolybenchC-pretune/${file#PolybenchC-tuned}"

        algorithm=$(echo "$file" | sed -e 's/.*\/\([^\/]*\)\.c/\1/')


        if ! diff -y --suppress-common-lines "$pretune" "$file"; then
            echo "$algorithm:" >> "$tmpdir/diff"
            # echo "diff -y --suppress-common-lines \"$pretune\" \"$file\":" >> "$tmpdir/diff"
        fi |
            grep -vE '^diff|^Only in' |
            grep -vE '^\s*int' |
            sed -e 's/^/\t/' |
            tee --append "$tmpdir/diff"
    done | wc -l)

    cat "$tmpdir/diff"

    echo "Changes in Polybench/C: $CHANGES"
}

measure_noarr() {
    find "PolybenchC-Noarr-tuned" -mindepth 3 -maxdepth 4 -type f -name "*.cpp" | sort | while read -r file; do
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

        printf "%s\0" "$file"
    done |
    xargs -0 awk '
BEGIN{
    last_filename = ""
    proto_structs = 0
}
/DEFINE_PROTO_STRUCT/ && # count proto structs (we use this macro so they are easy to find)
!/_layout/ && !/neutral_proto/{ # ignore layouts and neutral proto (they are there only for demonstration of generality)
    print "\t" $''0
    proto_structs++
}
FILENAME != last_filename {
    algorithm = last_filename = FILENAME

    sub(/.*\//, "", algorithm) # remove path
    sub(/\.cpp/, "", algorithm) # remove extension

    print algorithm ":"

}
END{
    print "Changes in Noarr: " proto_structs
}
'
}

measure_c_changes
measure_noarr
