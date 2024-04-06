#!/usr/bin/awk -f

BEGIN {
    splits = 0
    lines = 0
    line_sum = 0
}

{
    lines++
    line_sum += length($0)
}

# Each if, while, and for statement is a path
/\<if\>/ {
    splits++
}

/\<while\>/ {
    splits++
}

/\<for\>/ {
    splits++
}

# Each lambda is a path
/\[.*\].*\{/ {
    splits++
}

END {
    cyclomatic = splits
    printf "metric,cyclomatic complexity,%d\n", cyclomatic
    printf "metric,lines,%d\n", lines
    printf "metric,line_length,%f\n", line_sum / lines
}
