#!/usr/bin/awk -f

function reset() {
    delete operands
    delete operators

    operands["'i'"] = 0
    operands["'j'"] = 0
    operands["'k'"] = 0
    operands["'l'"] = 0
    operands["'m'"] = 0
    operands["'n'"] = 0
    operands["'o'"] = 0

    operands["'ni'"] = 0
    operands["'nj'"] = 0
    operands["'nk'"] = 0

    operands["'I'"] = 0
    operands["'J'"] = 0
    operands["'K'"] = 0
    operands["'L'"] = 0
    operands["'M'"] = 0
    operands["'N'"] = 0
    operands["'O'"] = 0

    operands["'a'"] = 0
    operands["'b'"] = 0
    operands["'c'"] = 0
    operands["'d'"] = 0
    operands["'e'"] = 0
    operands["'f'"] = 0
    operands["'g'"] = 0

    operands["'a1'"] = 0
    operands["'a2'"] = 0
    operands["'a3'"] = 0
    operands["'a4'"] = 0
    operands["'a5'"] = 0
    operands["'a6'"] = 0
    operands["'a7'"] = 0
    operands["'a8'"] = 0

    operands["'b1'"] = 0
    operands["'b2'"] = 0
    operands["'b3'"] = 0
    operands["'b4'"] = 0
    operands["'b5'"] = 0
    operands["'b6'"] = 0
    operands["'b7'"] = 0
    operands["'b8'"] = 0

    operands["'c1'"] = 0
    operands["'c2'"] = 0
    operands["'c3'"] = 0
    operands["'c4'"] = 0
    operands["'c5'"] = 0
    operands["'c6'"] = 0
    operands["'c7'"] = 0
    operands["'c8'"] = 0

    operands["'A'"] = 0
    operands["'B'"] = 0
    operands["'C'"] = 0
    operands["'D'"] = 0
    operands["'E'"] = 0
    operands["'F'"] = 0
    operands["'G'"] = 0

    operands["'A1'"] = 0
    operands["'A2'"] = 0
    operands["'A3'"] = 0
    operands["'A4'"] = 0

    operands["'B1'"] = 0
    operands["'B2'"] = 0
    operands["'B3'"] = 0
    operands["'B4'"] = 0

    operands["'C1'"] = 0
    operands["'C2'"] = 0
    operands["'C3'"] = 0
    operands["'C4'"] = 0

    operands["'DX'"] = 0
    operands["'DY'"] = 0
    operands["'DT'"] = 0

    operands["'s'"] = 0
    operands["'t'"] = 0
    operands["'u'"] = 0
    operands["'v'"] = 0

    operands["'u1'"] = 0
    operands["'u2'"] = 0
    operands["'v1'"] = 0
    operands["'v2'"] = 0

    operands["'ex'"] = 0
    operands["'ey'"] = 0
    operands["'hz'"] = 0

    operands["'table'"] = 0
    operands["'path'"] = 0
    operands["'tmp'"] = 0
    operands["'temp'"] = 0
    operands["'temp1'"] = 0
    operands["'temp2'"] = 0
    operands["'seq'"] = 0

    operands["'alpha'"] = 0
    operands["'beta'"] = 0
    operands["'eps'"] = 0

    operands["'data'"] = 0
    operands["'corr'"] = 0
    operands["'cov'"] = 0
    operands["'mean'"] = 0
    operands["'stddev'"] = 0
    operands["'max_score'"] = 0
    operands["'mul1'"] = 0
    operands["'mul2'"] = 0
    operands["'sum'" ] = 0
    operands["'nrm'" ] = 0
    operands["'norm'" ] = 0
    operands["'_fict_'"] = 0

    operands["'y1'"] = 0
    operands["'y2'"] = 0
    operands["'y_1'"] = 0
    operands["'y_2'"] = 0
    operands["'ym1'"] = 0
    operands["'ym2'"] = 0
    operands["'ym3'"] = 0
    operands["'yp1'"] = 0
    operands["'yp2'"] = 0
    operands["'yp3'"] = 0
    operands["'x1'"] = 0
    operands["'x2'"] = 0
    operands["'xm1'"] = 0
    operands["'xm2'"] = 0
    operands["'xm3'"] = 0
    operands["'xp1'"] = 0
    operands["'xp2'"] = 0
    operands["'xp3'"] = 0
    operands["'tm1'"] = 0
    operands["'tm2'"] = 0
    operands["'tm3'"] = 0
    operands["'tp1'"] = 0
    operands["'tp2'"] = 0
    operands["'tp3'"] = 0

    operands["'imgIn'"] = 0
    operands["'imgOut'"] = 0

    operands["'h'"] = 0
    operands["'x'"] = 0
    operands["'y'"] = 0
    operands["'z'"] = 0
    operands["'w'"] = 0

    operands["'p'"] = 0
    operands["'q'"] = 0
    operands["'r'"] = 0

    operands["'P'"] = 0
    operands["'Q'"] = 0
    operands["'R'"] = 0

    operands["'float_n'"] = 0

    operands["'tsteps'"] = 0
    operands["'TSTEPS'"] = 0

    operands["'_PB_TSTEPS'"] = 0
    operands["'_PB_TMAX'"] = 0

    operands["'_PB_N'"] = 0
    operands["'_PB_M'"] = 0

    operands["'_PB_H'"] = 0
    operands["'_PB_W'"] = 0

    operands["'_PB_NI'"] = 0
    operands["'_PB_NJ'"] = 0
    operands["'_PB_NK'"] = 0
    operands["'_PB_NL'"] = 0
    operands["'_PB_NM'"] = 0

    operands["'_PB_NY'"] = 0
    operands["'_PB_NZ'"] = 0
    operands["'_PB_NX'"] = 0

    operands["'_PB_NP'"] = 0
    operands["'_PB_NQ'"] = 0
    operands["'_PB_NR'"] = 0


    operators["'match'"] = 0

    operators["'SCALAR_VAL'"] = 0
    operators["'DATA_TYPE'"] = 0
    operators["'EXP_FUN'"] = 0
    operators["'SQRT_FUN'"] = 0
    operators["'POW_FUN'"] = 0


    operands["'std'"] = 0
    operands["'noarr'"] = 0

    operands["'state'"] = 0
    operands["'idx'"] = 0
    operands["'inner'"] = 0
    operands["'trav'"] = 0

    operands["'u_trans'"] = 0
    operands["'v_trans'"] = 0
    operands["'path_start_k'"] = 0
    operands["'path_end_k'"] = 0
    operands["'table_ik'"] = 0
    operands["'table_kj'"] = 0
    operands["'A_ik'"] = 0
    operands["'A_jk'"] = 0
    operands["'A_kj'"] = 0
    operands["'A_ij'"] = 0
    operands["'A_ji'"] = 0
    operands["'A_ii'"] = 0
    operands["'A_rqs'"] = 0
    operands["'A_renamed'"] = 0
    operands["'B_renamed'"] = 0
    operands["'C_renamed'"] = 0
    operands["'data_ki'"] = 0
    operands["'data_ji'"] = 0
    operands["'corr_ji'"] = 0
    operands["'cov_ji'"] = 0
    operands["'R_diag'"] = 0
    operands["'r_k'"] = 0
    operands["'y_k'"] = 0
    operands["'x_j'"] = 0
    operands["'seq_j'"] = 0

    operands["'order'"] = 0
    operands["'order1'"] = 0
    operands["'order2'"] = 0
    operands["'order3'"] = 0
    operands["'order4'"] = 0

    operands["'madd'"] = 0

    operators["'get_length'"] = 0
    operators["'get_index'"] = 0
    operators["'get_indices'"] = 0
    operators["'get_ref'"] = 0

    operators["'symmetric_spans'"] = 0
    operators["'symmetric_span'"] = 0
    operators["'reorder'"] = 0
    operators["'span'"] = 0
    operators["'shift'"] = 0
    operators["'reverse'"] = 0
    operators["'hoist'"] = 0
    operators["'fix'"] = 0

    operators["'for_each'"] = 0
    operators["'for_each_elem'"] = 0
    operators["'for_dims'"] = 0

    operators["'traverser'"] = 0
    operators["'planner'"] = 0

    operators["'planner_execute'"] = 0

    operators["'exp'"] = 0
    operators["'sqrt'"] = 0
    operators["'pow'"] = 0
    operators["'min'"] = 0

    operators["'num_t'"] = 0
}

function print_results() {
    if (use_total == 1) {
        delete operands
        for (operand in sum_operands)
            operands[operand] = sum_operands[operand]

        delete operators
        for (operator in sum_operators)
            operators[operator] = sum_operators[operator]

        maybe_total = "total_"
    } else {
        maybe_total = FILENAME  "_"
    }

    if (verbose == 2) {
        for (operand in operands) {
            if (operands[operand] > 0)
                printf "operand,%s,%s\n", operand, operands[operand]
        }

        for (operator in operators) {
            if (operators[operator] > 0)
                printf "operator,%s,%s\n", operator, operators[operator]
        }
    }

    distinct_operands = 0
    total_operands = 0
    for (operand in operands) {
        if (operands[operand] > 0)
            distinct_operands += 1

        total_operands += operands[operand]
    }

    distinct_operators = 0
    total_operators = 0
    for (operator in operators) {
        if (operators[operator] > 0)
            distinct_operators += 1

        total_operators += operators[operator]
    }

    if (verbose == 1) {
        printf "metric,distinct_operands,%s\n", distinct_operands
        printf "metric,total_operands,%s\n", total_operands
        printf "metric,distinct_operators,%s\n", distinct_operators
        printf "metric,total_operators,%s\n", total_operators
    }

    program_length = total_operands + total_operators
    program_vocabulary = distinct_operands + distinct_operators

    estimated_length = distinct_operands * log(distinct_operands) / log(2) + distinct_operators * log(distinct_operators) / log(2)
    volume = program_length * log(program_vocabulary) / log(2)
    if (distinct_operands == 0)
        difficulty = distinct_operators / 2
    else
        difficulty = distinct_operators / 2 * (total_operands / distinct_operands)
    effort = difficulty * volume

    subprogram_count++

    total_program_length += program_length
    total_program_vocabulary += program_vocabulary
    total_estimated_length += estimated_length
    total_volume += volume
    total_difficulty += difficulty
    total_effort += effort

    if (use_total == 1) {
        printf "metric,mean_program_length,%s\n", total_program_length / subprogram_count
        printf "metric,mean_program_vocabulary,%s\n", total_program_vocabulary / subprogram_count
        printf "metric,mean_estimated_length,%s\n", total_estimated_length / subprogram_count
        printf "metric,mean_volume,%s\n", total_volume / subprogram_count
        printf "metric,mean_difficulty,%s\n", total_difficulty / subprogram_count
        printf "metric,mean_effort,%s\n", total_effort / subprogram_count
    }

    printf "metric," maybe_total "program_length,%s\n", program_length
    printf "metric," maybe_total "program_vocabulary,%s\n", program_vocabulary
    printf "metric," maybe_total "estimated_length,%s\n", estimated_length
    printf "metric," maybe_total "volume,%s\n", volume
    printf "metric," maybe_total "difficulty,%s\n", difficulty
    printf "metric," maybe_total "effort,%s\n", effort

    if (unknown_identifiers > 0)
        print "Unknown identifiers:", unknown_identifiers > "/dev/stderr"
    if (unknown_tokens > 0)
        print "Unknown tokens:", unknown_tokens > "/dev/stderr"

    if (unknown_identifiers > 0 || unknown_tokens > 0) {
        print "Error: unknown identifiers or tokens, adjust the script to handle them" > "/dev/stderr"
        exit 1
    }
    
    reset()
}


BEGIN {
    FS = " "
    OFS = " "

    delete sum_operands
    delete sum_operators

    reset()
}

# Numeric constants are operands
/^numeric_constant/ {
    operands[$2] += 1
    sum_operands[$2] += 1
    next
}

# Character constants are operands
/^char_constant/ {
    operands[$2] += 1
    sum_operands[$2] += 1
    next
}

/^identifier/ {
    if ($2 in operands) {
        operands[$2] += 1
        sum_operands[$2] += 1
    } else if ($2 in operators) {
        operators[$2] += 1
        sum_operators[$2] += 1
    } else {
        unknown_identifiers += 1
        print "Unknown identifier: " $2 > "/dev/stderr"
    }
    next
}
# EOF is neither an operand nor an operator
/^eof/ {
    print_results()
    next
}

# Operators
/^(r_square|r_paren|l_square|l_paren|plus|minus|semi|lessequal|for|equal|star|less|l_brace|r_brace|slash|greaterequal|comma|if|else|question|ampamp|colon|greater|caret|pipe|amp|period|auto|const)/ {
    operators[$2] += 1
    sum_operators[$2] += 1
    next
}

{
    unknown_tokens += 1
    print "Unknown token: " $2 > "/dev/stderr"
}

END {
    use_total = 1

    print_results()
}
