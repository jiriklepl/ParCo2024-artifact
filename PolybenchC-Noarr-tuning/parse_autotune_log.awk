#!/usr/bin/awk -f

BEGIN {
    algorithm = "Unknown"

    delete compile_time
    delete run_time
    delete compile_returncode
    delete run_returncode
    delete instances
}

/^Autotuning / {
    algorithm = $2
}

/Compile time/ {
    instance = instances[algorithm]++
    compile_time[algorithm][instance] = $NF
}

/Compile returncode/ {
    compile_returncode[algorithm][instance] = $NF
}

/Run time/ {
    run_time[algorithm][instance] = $NF
}

/Run returncode/ {
    run_returncode[algorithm][instance] = $NF
}

END {
    for (algorithm in compile_time) {
        for (instance in compile_time[algorithm]) {
            printf "compile_time,%s,%d,%f\n", algorithm, instance, compile_time[algorithm][instance]
        }
        for (instance in run_time[algorithm]) {
            printf "run_time,%s,%d,%f\n", algorithm, instance, run_time[algorithm][instance]
        }
        for (instance in compile_returncode[algorithm]) {
            printf "compile_returncode,%s,%d,%d\n", algorithm, instance, compile_returncode[algorithm][instance]
        }
        for (instance in run_returncode[algorithm]) {
            printf "run_returncode,%s,%d,%d\n", algorithm, instance, run_returncode[algorithm][instance]
        }
    }
}
