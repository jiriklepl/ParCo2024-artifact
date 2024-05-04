
traverser(data, mean) | for_dims<'j'>([=](auto inner) {
    mean[inner] = 0;
    inner | [=](auto state) { mean[state] += data[state]; };
    mean[inner] /= float_n;
});
traverser(data, mean, stddev) | for_dims<'j'>([=](auto inner) {
    stddev[inner] = 0;
    inner | [=](auto state) { stddev[state] += (data[state] - mean[state]) * (data[state] - mean[state]); };
    stddev[inner] /= float_n;
    stddev[inner] = std::sqrt(stddev[inner]);
    stddev[inner] = stddev[inner] <= eps ? (num_t)1.0 : stddev[inner];
});
traverser(data, mean, stddev) | [=](auto state) {
    data[state] -= mean[state];
    data[state] /= std::sqrt(float_n) * stddev[state];
};
traverser(data, corr, data_ki, corr_ji) ^ span<'i'>(0, ni - 1) | for_dims<'i'>([=](auto inner) {
    auto i = get_index<'i'>(inner);
    corr[inner.state() & idx<'j'>(i)] = 1;
    inner ^ shift<'j'>(i + 1) | for_dims<'j'>([=](auto inner) {
        corr[inner] = 0;
        inner | [=](auto state) { corr[state] += data_ki[state] * data[state]; };
        corr_ji[inner] = corr[inner];
    });
});
corr[idx<'i'>(ni - 1) & idx<'j'>(ni - 1)] = 1;
traverser(mean) | [=](auto state) { mean[state] = 0; };
traverser(data, mean) | [=](auto state) { mean[state] += data[state]; };
traverser(mean) | [=](auto state) { mean[state] /= float_n; };
traverser(data, mean) | [=](auto state) { data[state] -= mean[state]; };
traverser(data, cov, data_ki, cov_ji) | for_dims<'i'>([=](auto inner) {
    inner ^ shift<'j'>(get_index<'i'>(inner)) | for_dims<'j'>([=](auto inner) {
        cov[inner] = 0;
        inner | [=](auto state) { cov[state] += data[state] * data_ki[state]; };
        cov[inner] /= float_n - (num_t)1;
        cov_ji[inner] = cov[inner];
    });
});
traverser(C, A, B) | for_dims<'i'>([=](auto inner) {
    inner | for_each<'j'>([=](auto state) { C[state] *= beta; });
    inner | [=](auto state) { C[state] += alpha * A[state] * B[state]; };
});
traverser(A, u1, u2, v1, v2) ^ order1 |
    [=](auto state) { A[state] = A[state] + u1[state] * v1[state] + u2[state] * v2[state]; };
traverser(x, A_ji, y) ^ order2 | [=](auto state) { x[state] = x[state] + beta * A_ji[state] * y[state]; };
traverser(x, z) | [=](auto state) { x[state] = x[state] + z[state]; };
traverser(A, w, x_j) ^ order3 | [=](auto state) { w[state] = w[state] + alpha * A[state] * x_j[state]; };
traverser(A, B, tmp, x, y) | for_dims<'i'>([=](auto inner) {
    tmp[inner] = 0;
    y[inner] = 0;
    inner | [=](auto state) {
        tmp[state] += A[state] * x[state];
        y[state] += B[state] * x[state];
    };
    y[inner] = alpha * tmp[inner] + beta * y[inner];
});
planner(C, A, B) ^ for_dims<'i', 'j'>([=](auto inner) {
    num_t temp = 0;
    inner ^ span<'k'>(get_index<'i'>(inner)) ^ for_each([=, &temp](auto state) {
        C_renamed[state] += alpha * B[state] * A[state];
        temp += B_renamed[state] * A[state];
    }) | planner_execute();
    C[inner] = beta * C[inner] + alpha * B[inner] * A[inner.state() & idx<'k'>(get_index<'i'>(inner))] + alpha * temp;
}) ^ order
    | planner_execute();
traverser(C, A, B, A_renamed, B_renamed) | for_dims<'i'>([=](auto inner) {
    auto i = get_index<'i'>(inner);
    inner ^ span<'j'>(i + 1) | for_dims<'j'>([=](auto inner) { C[inner] *= beta; });
    inner ^ span<'j'>(i + 1) ^ order |
        [=](auto state) { C[state] += A_renamed[state] * alpha * B[state] + B_renamed[state] * alpha * A[state]; };
});
traverser(C, A, A_renamed) | for_dims<'i'>([=](auto inner) {
    auto i = get_index<'i'>(inner);
    inner ^ span<'j'>(i + 1) | for_dims<'j'>([=](auto inner) { C[inner] *= beta; });
    inner ^ span<'j'>(i + 1) ^ order | [=](auto state) { C[state] += alpha * A[state] * A_renamed[state]; };
});
planner(A, B, B_renamed) ^ for_each_elem([](auto &&A, auto &&B, auto &&B_renamed) {
    B += A * B_renamed;
}) ^ for_dims<'i', 'j'>([=](auto inner) {
    inner ^ shift<'k'>(get_index<'i'>(inner) + 1) | planner_execute();
    B[inner] *= alpha;
}) ^ order
    | planner_execute();
planner(tmp, A, B) ^ for_each_elem([=](auto &&tmp, auto &&A, auto &&B) {
    tmp += alpha * A * B;
}) ^ for_dims<'i', 'j'>([=](auto inner) {
    tmp[inner] = 0;
    inner();
}) ^ order1
    | planner_execute();
planner(D, tmp, C) ^ for_each_elem([](auto &&D, auto &&tmp, auto &&C) {
    D += tmp * C;
}) ^ for_dims<'i', 'l'>([=](auto inner) {
    D[inner] *= beta;
    inner();
}) ^ order2
    | planner_execute();
planner(E, A, B) ^ madd ^ for_dims<'i', 'j'>([=](auto inner) {
    E[inner] = 0;
    inner();
}) ^ order1
    | planner_execute();
planner(F, C, D) ^ madd ^ for_dims<'j', 'l'>([=](auto inner) {
    F[inner] = 0;
    inner();
}) ^ order2
    | planner_execute();
planner(G, E, F) ^ madd ^ for_dims<'i', 'l'>([=](auto inner) {
    G[inner] = 0;
    inner();
}) ^ order3
    | planner_execute();
traverser(y) | [=](auto state) { y[state] = 0; };
traverser(tmp, A, x, y) | for_dims<'i'>([=](auto inner) {
    tmp[inner] = 0;
    inner | [=](auto state) { tmp[state] += A[state] * x[state]; };
    inner | [=](auto state) { y[state] += A[state] * tmp[state]; };
});
traverser(s) | [=](auto state) { s[state] = 0; };
planner(A, s, q, p, r) ^ for_each_elem([](auto &&A, auto &&s, auto &&q, auto &&p, auto &&r) {
    s += A * r;
    q += A * p;
}) ^ for_dims<'i'>([=](auto inner) {
    q[inner] = 0;
    inner();
}) ^ order
    | planner_execute();
planner(A, C4, sum) ^ for_dims<'r', 'q'>([=](auto inner) {
    inner ^ for_dims<'p'>([=](auto inner) {
        sum[inner] = 0;
        inner ^ for_each([=](auto state) { sum[state] += A_rqs[state] * C4[state]; }) | planner_execute();
    }) | planner_execute();
    inner ^ for_dims<'p'>([=](auto inner) { A[inner] = sum[inner]; }) | planner_execute();
}) ^ order
    | planner_execute();
traverser(x1, A, y1) ^ order1 | [=](auto state) { x1[state] += A[state] * y1[state]; };
traverser(x2, A_ji, y2) ^ order2 | [=](auto state) { x2[state] += A_ji[state] * y2[state]; };
traverser(A, A_ik, A_jk) | for_dims<'i'>([=](auto inner) {
    auto i = get_index<'i'>(inner);
    inner ^ span<'j'>(i) | for_dims<'j'>([=](auto inner) {
        auto j = get_index<'j'>(inner);
        inner ^ span<'k'>(j) | [=](auto state) { A[state] -= A_ik[state] * A_jk[state]; };
        A[inner] /= (A ^ fix<'i'>(j))[inner];
    });
    auto A_ii = A ^ fix<'j'>(i);
    inner ^ span<'k'>(i) | for_each<'k'>([=](auto state) { A_ii[state] -= A_ik[state] * A_ik[state]; });
    A_ii[inner] = std::sqrt(A_ii[inner]);
});
y[idx<'i'>(0)] = -r[idx<'i'>(0)];
beta = 1;
alpha = -r[idx<'i'>(0)];
traverser(r, y, r_k, y_k) ^ shift<'k'>(1) | for_dims<'k'>([=, &alpha, &beta, &sum, z = z.get_ref()](auto inner) {
    beta = (1 - alpha * alpha) * beta;
    sum = 0;
    auto traverser = inner ^ span<'i'>(get_index<'k'>(inner));
    traverser | [=, &sum](auto state) {
        auto [i, k] = get_indices<'i', 'k'>(state);
        sum += r[idx<'i'>(k - i - 1)] * y[state];
    };
    alpha = -(r_k[inner] + sum) / beta;
    traverser | [=, &alpha](auto state) {
        auto [i, k] = get_indices<'i', 'k'>(state);
        z[state] = y[state] + alpha * y[idx<'i'>(k - i - 1)];
    };
    traverser | [=](auto state) { y[state] = z[state]; };
    y_k[inner] = alpha;
});
traverser(A_ij, R, Q) | for_dims<'k'>([=](auto inner) {
    auto k = get_index<'k'>(inner);
    num_t norm = 0;
    inner | for_each<'i'>([=, &norm](auto state) { norm += A[state] * A[state]; });
    auto R_diag = R ^ fix<'j'>(k);
    R_diag[inner] = std::sqrt(norm);
    inner | for_each<'i'>([=](auto state) { Q[state] = A[state] / R_diag[state]; });
    inner ^ shift<'j'>(k + 1) | for_dims<'j'>([=](auto inner) {
        R[inner] = 0;
        inner | [=](auto state) { R[state] = R[state] + Q[state] * A_ij[state]; };
        inner | [=](auto state) { A_ij[state] = A_ij[state] - Q[state] * R[state]; };
    });
});
traverser(A, b, x, y, A_ik, A_kj) | for_dims<'i'>([=](auto inner) {
    auto i = get_index<'i'>(inner);
    inner ^ span<'j'>(i) | for_dims<'j'>([=](auto inner) {
        auto j = get_index<'j'>(inner);
        num_t w = A[inner];
        inner ^ span<'k'>(j) | [=, &w](auto state) { w -= A_ik[state] * A_kj[state]; };
        A[inner] = w / (A ^ fix<'i'>(j))[inner];
    });
    inner ^ shift<'j'>(i) | for_dims<'j'>([=](auto inner) {
        num_t w = A[inner];
        inner ^ span<'k'>(i) | [=, &w](auto state) { w -= A_ik[state] * A_kj[state]; };
        A[inner] = w;
    });
});
traverser(A, b, y) | for_dims<'i'>([=](auto inner) {
    num_t w = b[inner];
    inner ^ span<'j'>(get_index<'i'>(inner)) |
        for_each<'j'>([=, &w](auto state) { w -= A[state] * y[idx<'i'>(get_index<'j'>(state))]; });
    y[inner] = w;
});
traverser(A, x) ^ reverse<'i'>() | for_dims<'i'>([=](auto inner) {
    auto i = get_index<'i'>(inner);
    num_t w = y[inner];
    inner ^ shift<'j'>(i + 1) |
        for_each<'j'>([=, &w](auto state) { w -= A[state] * x[idx<'i'>(get_index<'j'>(state))]; });
    x[inner] = w / A[inner.state() & idx<'j'>(i)];
});
traverser(A, A_ik, A_kj) | for_dims<'i'>([=](auto inner) {
    auto i = get_index<'i'>(inner);
    inner ^ span<'j'>(i) | for_dims<'j'>([=](auto inner) {
        auto j = get_index<'j'>(inner);
        inner ^ span<'k'>(j) | [=](auto state) { A[state] -= A_ik[state] * A_kj[state]; };
        A[inner] /= (A ^ fix<'i'>(j))[inner];
    });
    inner ^ shift<'j'>(i) ^ span<'k'>(i) ^ order | [=](auto state) { A[state] -= A_ik[state] * A_kj[state]; };
});
traverser(L, x, b) | for_dims<'i'>([=](auto inner) {
    auto i = get_index<'i'>(inner);
    x[inner] = b[inner];
    inner ^ span<'j'>(i) | [=](auto state) { x[state] -= L[state] * x_j[state]; };
    x[inner] = x[inner] / L[inner.state() & idx<'j'>(i)];
});
k = ((num_t)1.0 - std::exp(-alpha)) * ((num_t)1.0 - std::exp(-alpha)) /
    ((num_t)1.0 + (num_t)2.0 * alpha * std::exp(-alpha) - std::exp(((num_t)2.0 * alpha)));
a1 = a5 = k;
a2 = a6 = k * std::exp(-alpha) * (alpha - (num_t)1.0);
a3 = a7 = k * std::exp(-alpha) * (alpha + (num_t)1.0);
a4 = a8 = -k * std::exp(((num_t)(-2.0) * alpha));
b1 = std::pow((num_t)2.0, -alpha);
b2 = -std::exp(((num_t)(-2.0) * alpha));
c1 = c2 = 1;
traverser(imgIn, y1) | for_dims<'w'>([=](auto inner) {
    num_t ym1 = 0;
    num_t ym2 = 0;
    num_t xm1 = 0;
    inner | [&, y1, imgIn](auto state) {
        y1[state] = a1 * imgIn[state] + a2 * xm1 + b1 * ym1 + b2 * ym2;
        xm1 = imgIn[state];
        ym2 = ym1;
        ym1 = y1[state];
    };
});
traverser(imgIn, y2) | for_dims<'w'>([=](auto inner) {
    num_t yp1 = 0;
    num_t yp2 = 0;
    num_t xp1 = 0;
    num_t xp2 = 0;
    inner ^ reverse<'h'>() | [&, y2, imgIn](auto state) {
        y2[state] = a3 * xp1 + a4 * xp2 + b1 * yp1 + b2 * yp2;
        xp2 = xp1;
        xp1 = imgIn[state];
        yp2 = yp1;
        yp1 = y2[state];
    };
});
traverser(y1, y2, imgOut) | [=](auto state) { imgOut[state] = c1 * (y1[state] + y2[state]); };
traverser(imgOut, y1) | for_dims<'h'>([=](auto inner) {
    num_t tm1 = 0;
    num_t ym1 = 0;
    num_t ym2 = 0;
    inner | [&, y1, imgOut](auto state) {
        y1[state] = a5 * imgOut[state] + a6 * tm1 + b1 * ym1 + b2 * ym2;
        tm1 = imgOut[state];
        ym2 = ym1;
        ym1 = y1[state];
    };
});
traverser(imgOut, y2) | for_dims<'h'>([=](auto inner) {
    num_t tp1 = 0;
    num_t tp2 = 0;
    num_t yp1 = 0;
    num_t yp2 = 0;
    inner ^ reverse<'w'>() | [&, y2, imgOut](auto state) {
        y2[state] = a7 * tp1 + a8 * tp2 + b1 * yp1 + b2 * yp2;
        tp2 = tp1;
        tp1 = imgOut[state];
        yp2 = yp1;
        yp1 = y2[state];
    };
});
traverser(y1, y2, imgOut) | [=](auto state) { imgOut[state] = c2 * (y1[state] + y2[state]); };
traverser(path, path_start_k, path_end_k) ^ hoist<'k'>() ^ order |
    [=](auto state) { path[state] = std::min(path_start_k[state] + path_end_k[state], path[state]); };
traverser(seq, table, table_ik, table_kj) ^ reverse<'i'>() | for_dims<'i'>([=](auto inner) {
    auto i = get_index<'i'>(inner);
    inner ^ shift<'j'>(i + 1) | for_dims<'j'>([=](auto inner) {
        auto state = inner.state();
        auto [i, j] = get_indices<'i', 'j'>(state);
        auto ni = table | get_length<'i'>();
        if (j >= 0)
            table[state] = max_score(table[state], table[state - idx<'j'>(1)]);
        if (i + 1 < ni)
            table[state] = max_score(table[state], table[state + idx<'i'>(1)]);
        if (j >= 0 || i + 1 < ni) {
            if (i < j - 1)
                table[state] =
                    max_score(table[state], table[state + idx<'i'>(1) - idx<'j'>(1)] + match(seq[state], seq_j[state]));
            else
                table[state] = max_score(table[state], table[state + idx<'i'>(1) - idx<'j'>(1)]);
        }
        inner ^ span<'k'>(i + 1, j) | [=](auto state) {
            table[state] = max_score(table[state], table_ik[state] + table_kj[state + idx<'k'>(1)]);
        };
    });
});

num_t DX = (num_t)1.0 / (u | get_length<'i'>());
num_t DY = (num_t)1.0 / (u | get_length<'j'>());
num_t DT = (num_t)1.0 / tsteps;
num_t B1 = 2.0;
num_t B2 = 1.0;
num_t mul1 = B1 * DT / (DX * DX);
num_t mul2 = B2 * DT / (DY * DY);
num_t a = -mul1 / (num_t)2.0;
num_t b = (num_t)1.0 + mul1;
num_t c = a;
num_t d = -mul2 / (num_t)2.0;
num_t e = (num_t)1.0 + mul2;
num_t f = d;
trav ^ symmetric_spans<'i', 'j'>(u, 1, 1) | for_dims<'t'>([=](auto inner) {
    inner | for_dims<'i'>([=](auto inner) {
        auto state = inner.state();
        v[state & idx<'j'>(0)] = (num_t)1.0;
        p[state & idx<'j'>(0)] = (num_t)0.0;
        q[state & idx<'j'>(0)] = v[state & idx<'j'>(0)];
        inner | [=](auto state) {
            p[state] = -c / (a * p[state - idx<'j'>(1)] + b);
            q[state] = (-d * u_trans[state - idx<'i'>(1)] + (B2 + B1 * d) * u_trans[state] -
                        f * u_trans[state + idx<'i'>(1)] - a * q[state - idx<'j'>(1)]) /
                       (a * p[state - idx<'j'>(1)] + b);
        };
        v[state & idx<'j'>((v | get_length<'j'>()) - 1)] = (num_t)1.0;
        inner ^ reverse<'j'>() | [=](auto state) { v[state] = p[state] * v[state + idx<'j'>(1)] + q[state]; };
    });
    inner | for_dims<'i'>([=](auto inner) {
        auto state = inner.state();
        u[state & idx<'j'>(0)] = (num_t)1.0;
        p[state & idx<'j'>(0)] = (num_t)0.0;
        q[state & idx<'j'>(0)] = u[state & idx<'j'>(0)];
        inner | [=](auto state) {
            p[state] = -f / (d * p[state - idx<'j'>(1)] + e);
            q[state] = (-a * v_trans[state - idx<'i'>(1)] + (B2 + B1 * a) * v_trans[state] -
                        c * v_trans[state + idx<'i'>(1)] - d * q[state - idx<'j'>(1)]) /
                       (d * p[state - idx<'j'>(1)] + e);
        };
        u[state & idx<'j'>((u | get_length<'j'>()) - 1)] = (num_t)1.0;
        inner ^ reverse<'j'>() | [=](auto state) { u[state] = p[state] * u[state + idx<'j'>(1)] + q[state]; };
    });
});
traverser(ex, ey, hz, _fict_) | for_dims<'t'>([=](auto inner) {
    inner ^ shift<'i'>(1) | for_each<'j'>([=](auto state) { ey[state & idx<'i'>(0)] = _fict_[state]; });
    inner ^ shift<'i'>(1) |
        [=](auto state) { ey[state] = ey[state] - (num_t).5 * (hz[state] - hz[state - idx<'i'>(1)]); };
    inner ^ shift<'j'>(1) |
        [=](auto state) { ex[state] = ex[state] - (num_t).5 * (hz[state] - hz[state - idx<'j'>(1)]); };
    inner ^ span<'i'>(0, (ex | get_length<'i'>()) - 1) ^ span<'j'>(0, (ex | get_length<'j'>()) - 1) | [=](auto state) {
        hz[state] = hz[state] - (num_t).7 * (ex[state + idx<'j'>(1)] - ex[state] + ey[state + idx<'i'>(1)] - ey[state]);
    };
});
trav ^ symmetric_spans<'i', 'j', 'k'>(A, 1, 1, 1) ^ order | for_dims<'t'>([=](auto inner) {
    inner | [=](auto state) {
        B[state] = (num_t).125 * (A[state - idx<'i'>(1)] - 2 * A[state] + A[state + idx<'i'>(1)]) +
                   (num_t).125 * (A[state - idx<'j'>(1)] - 2 * A[state] + A[state + idx<'j'>(1)]) +
                   (num_t).125 * (A[state - idx<'k'>(1)] - 2 * A[state] + A[state + idx<'k'>(1)]) + A[state];
    };
    inner | [=](auto state) {
        A[state] = (num_t).125 * (B[state - idx<'i'>(1)] - 2 * B[state] + B[state + idx<'i'>(1)]) +
                   (num_t).125 * (B[state - idx<'j'>(1)] - 2 * B[state] + B[state + idx<'j'>(1)]) +
                   (num_t).125 * (B[state - idx<'k'>(1)] - 2 * B[state] + B[state + idx<'k'>(1)]) + B[state];
    };
});
trav | for_dims<'t'>([=](auto inner) {
    inner ^ symmetric_span<'i'>(B, 1) |
        [=](auto state) { B[state] = 0.33333 * (A[state - idx<'i'>(1)] + A[state] + A[state + idx<'i'>(1)]); };
    inner ^ symmetric_span<'i'>(A, 1) |
        [=](auto state) { A[state] = 0.33333 * (B[state - idx<'i'>(1)] + B[state] + B[state + idx<'i'>(1)]); };
});
trav ^ symmetric_spans<'i', 'j'>(A, 1, 1) ^ order | for_dims<'t'>([=](auto inner) {
    inner | [=](auto state) {
        B[state] = (num_t).2 * (A[state] + A[state - idx<'j'>(1)] + A[state + idx<'j'>(1)] + A[state + idx<'i'>(1)] +
                                A[state - idx<'i'>(1)]);
    };
    inner | [=](auto state) {
        A[state] = (num_t).2 * (B[state] + B[state - idx<'j'>(1)] + B[state + idx<'j'>(1)] + B[state + idx<'i'>(1)] +
                                B[state - idx<'i'>(1)]);
    };
});
trav ^ symmetric_spans<'i', 'j'>(A, 1, 1) ^ reorder<'t', 'i', 'j'>() | [=](auto state) {
    A[state] = (A[state - idx<'i'>(1) - idx<'j'>(1)] + A[state - idx<'i'>(1)] + A[state - idx<'i'>(1) + idx<'j'>(1)] +
                A[state - idx<'j'>(1)] + A[state] + A[state + idx<'j'>(1)] + A[state + idx<'i'>(1) - idx<'j'>(1)] +
                A[state + idx<'i'>(1)] + A[state + idx<'i'>(1) + idx<'j'>(1)]) /
               (num_t)9.0;
};
