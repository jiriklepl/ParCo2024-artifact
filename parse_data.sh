#!/bin/bash

echo "name,implementation,time"

find data -name "*.log" |
while read -r file; do
    (awk -vfile="$(basename "$file" | sed 's/\..*$//')" '
/C:/ {
    if (C++) data_c[C] = $2
}

/Noarr:/ {
    if (Noarr++) data_noarr[Noarr] = $2
}

END {
    for (i = 2; i <= (C > Noarr ? C : Noarr); i++) {
        if (data_noarr[i] > 0)
            print(file ",noarr," data_noarr[i])

        if (data_c[i] > 0)
            print(file ",c," data_c[i])
    }
}' "$file" & wait )

done
