
noarr::traverser(data, mean).template for_dims<'j'>([=](auto inner) {
    auto state = inner.state();
    mean[state] = 0;
    inner.for_each([=](auto state) { mean[state] += data[state]; });
    mean[state] /= float_n;
});
noarr::traverser(data, mean, stddev).template for_dims<'j'>([=](auto inner) {
    auto state = inner.state();
    stddev[state] = 0;
    inner.for_each([=](auto state) { stddev[state] += (data[state] - mean[state]) * (data[state] - mean[state]); });
    stddev[state] /= float_n;
    stddev[state] = std::sqrt(stddev[state]);
    stddev[state] = stddev[state] <= eps ? (num_t)1.0 : stddev[state];
});
noarr::traverser(data, mean, stddev).for_each([=](auto state) {
    data[state] -= mean[state];
    data[state] /= std::sqrt(float_n) * stddev[state];
});
auto traverser = noarr::traverser(data, corr, data_ki, corr_ji);
traverser.order(noarr::span<'i'>(0, ni - 1)).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    auto i = noarr::get_index<'i'>(state);
    corr[state & noarr::idx<'j'>(i)] = 1;
    inner.order(noarr::shift<'j'>(i + 1)).template for_dims<'j'>([=](auto inner) {
        auto state = inner.state();
        corr[state] = 0;
        inner.for_each([=](auto state) { corr[state] += data_ki[state] * data[state]; });
        corr_ji[state] = corr[state];
    });
});
corr[noarr::idx<'i'>(ni - 1) & noarr::idx<'j'>(ni - 1)] = 1;
noarr::traverser(mean).for_each([=](auto state) { mean[state] = 0; });
noarr::traverser(data, mean).for_each([=](auto state) { mean[state] += data[state]; });
noarr::traverser(mean).for_each([=](auto state) { mean[state] /= float_n; });
noarr::traverser(data, mean).for_each([=](auto state) { data[state] -= mean[state]; });
noarr::traverser(data, cov, data_ki, cov_ji).template for_dims<'i'>([=](auto inner) {
    inner.order(noarr::shift<'j'>(noarr::get_index<'i'>(inner.state()))).template for_dims<'j'>([=](auto inner) {
        auto state = inner.state();
        cov[state] = 0;
        inner.for_each([=](auto state) { cov[state] += data[state] * data_ki[state]; });
        cov[state] /= float_n - (num_t)1;
        cov_ji[state] = cov[state];
    });
});
noarr::traverser(C, A, B).template for_dims<'i'>([=](auto inner) {
    inner.template for_each<'j'>([=](auto state) { C[state] *= beta; });
    inner.for_each([=](auto state) { C[state] += alpha * A[state] * B[state]; });
});
noarr::traverser(A, u1, u2, v1, v2).order(order1).for_each([=](auto state) {
    A[state] = A[state] + u1[state] * v1[state] + u2[state] * v2[state];
});
noarr::traverser(x, A_ji, y).order(order2).for_each([=](auto state) {
    x[state] = x[state] + beta * A_ji[state] * y[state];
});
noarr::traverser(x, z).for_each([=](auto state) { x[state] = x[state] + z[state]; });
noarr::traverser(A, w, x_j).order(order3).for_each([=](auto state) {
    w[state] = w[state] + alpha * A[state] * x_j[state];
});
noarr::traverser(A, B, tmp, x, y).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    tmp[state] = 0;
    y[state] = 0;
    inner.for_each([=](auto state) {
        tmp[state] += A[state] * x[state];
        y[state] += B[state] * x[state];
    });
    y[state] = alpha * tmp[state] + beta * y[state];
});
noarr::planner(C, A, B)
    .template for_sections<'i', 'j'>([=](auto inner) {
        num_t temp = 0;
        auto state = inner.state();
        inner.order(noarr::slice<'k'>(0, noarr::get_index<'i'>(state))).for_each([=, &temp](auto state) {
            C_renamed[state] += alpha * B[state] * A[state];
            temp += B_renamed[state] * A[state];
        })();
        C[state] = beta * C[state] + alpha * B[state] * A[state & noarr::idx<'k'>(noarr::get_index<'i'>(state))] +
                   alpha * temp;
    })
    .order(noarr::hoist<'j'>())
    .order(noarr::hoist<'i'>())
    .order(order)();
noarr::traverser(C, A, B, A_renamed, B_renamed).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    inner.order(noarr::slice<'j'>(0, noarr::get_index<'i'>(state) + 1)).template for_dims<'j'>([=](auto inner) {
        C[inner.state()] *= beta;
    });
    inner.order(noarr::slice<'j'>(0, noarr::get_index<'i'>(state) + 1)).order(order).for_each([=](auto state) {
        C[state] += A_renamed[state] * alpha * B[state] + B_renamed[state] * alpha * A[state];
    });
});
noarr::traverser(C, A, A_renamed).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    inner.order(noarr::slice<'j'>(0, noarr::get_index<'i'>(state) + 1)).template for_dims<'j'>([=](auto inner) {
        C[inner.state()] *= beta;
    });
    inner.order(noarr::slice<'j'>(0, noarr::get_index<'i'>(state) + 1)).order(order).for_each([=](auto state) {
        C[state] += alpha * A[state] * A_renamed[state];
    });
});
noarr::planner(A, B, B_renamed)
    .for_each_elem([](auto &&A, auto &&B, auto &&B_renamed) { B += A * B_renamed; })
    .template for_sections<'i', 'j'>([=](auto inner) {
        auto state = inner.state();
        inner.order(noarr::shift<'k'>(noarr::get_index<'i'>(state) + 1))();
        B[state] *= alpha;
    })
    .order(noarr::hoist<'j'>())
    .order(noarr::hoist<'i'>())
    .order(order)();
noarr::planner(tmp, A, B)
    .for_each_elem([alpha](auto &&tmp, auto &&A, auto &&B) { tmp += alpha * A * B; })
    .template for_sections<'i', 'j'>([tmp](auto inner) {
        auto state = inner.state();
        tmp[state] = 0;
        inner();
    })
    .order(noarr::hoist<'j'>())
    .order(noarr::hoist<'i'>())
    .order(order1)();
noarr::planner(D, tmp, C)
    .for_each_elem([](auto &&D, auto &&tmp, auto &&C) { D += tmp * C; })
    .template for_sections<'i', 'l'>([D, beta](auto inner) {
        auto state = inner.state();
        D[state] *= beta;
        inner();
    })
    .order(noarr::hoist<'l'>())
    .order(noarr::hoist<'i'>())
    .order(order2)();
noarr::planner(E, A, B)
    .for_each_elem(madd)
    .template for_sections<'i', 'j'>([=](auto inner) {
        E[inner.state()] = 0;
        inner();
    })
    .order(noarr::hoist<'k'>())
    .order(noarr::hoist<'j'>())
    .order(noarr::hoist<'i'>())
    .order(order1)();
noarr::planner(F, C, D)
    .for_each_elem(madd)
    .template for_sections<'j', 'l'>([=](auto inner) {
        F[inner.state()] = 0;
        inner();
    })
    .order(noarr::hoist<'m'>())
    .order(noarr::hoist<'l'>())
    .order(noarr::hoist<'j'>())
    .order(order2)();
noarr::planner(G, E, F)
    .for_each_elem(madd)
    .template for_sections<'i', 'l'>([=](auto inner) {
        G[inner.state()] = 0;
        inner();
    })
    .order(noarr::hoist<'j'>())
    .order(noarr::hoist<'l'>())
    .order(noarr::hoist<'i'>())
    .order(order3)();
noarr::traverser(y).for_each([=](auto state) { y[state] = 0; });
noarr::traverser(tmp, A, x, y).template for_dims<'i'>([=](auto inner) {
    tmp[inner.state()] = 0;
    inner.for_each([=](auto state) { tmp[state] += A[state] * x[state]; });
    inner.for_each([=](auto state) { y[state] += A[state] * tmp[state]; });
});
noarr::traverser(s).for_each([=](auto state) { s[state] = 0; });
noarr::planner(A, s, q, p, r)
    .for_each_elem([](auto &&A, auto &&s, auto &&q, auto &&p, auto &&r) {
        s += A * r;
        q += A * p;
    })
    .template for_sections<'i'>([=](auto inner) {
        auto state = inner.state();
        q[state] = 0;
        inner();
    })
    .order(noarr::hoist<'i'>())
    .order(order)();
noarr::planner(A, C4, sum)
    .template for_sections<'r', 'q'>([=](auto inner) {
        inner
            .template for_sections<'p'>([=](auto inner) {
                auto state = inner.state();
                sum[state] = 0;
                inner.for_each([=](auto state) { sum[state] += A_rqs[state] * C4[state]; })();
            })
            .order(noarr::hoist<'p'>())();
        inner
            .template for_sections<'p'>([=](auto inner) {
                auto state = inner.state();
                A[state] = sum[state];
            })
            .order(noarr::hoist<'p'>())();
    })
    .order(noarr::hoist<'q'>())
    .order(noarr::hoist<'r'>())
    .order(order)();
noarr::traverser(x1, A, y1).order(order1).for_each([=](auto state) { x1[state] += A[state] * y1[state]; });
noarr::traverser(x2, A_ji, y2).order(order2).for_each([=](auto state) { x2[state] += A_ji[state] * y2[state]; });
noarr::traverser(A, A_ik, A_jk).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    inner.order(noarr::slice<'j'>(0, noarr::get_index<'i'>(state))).template for_dims<'j'>([=](auto inner) {
        auto state = inner.state();
        inner.order(noarr::slice<'k'>(0, noarr::get_index<'j'>(state))).for_each([=](auto state) {
            A[state] -= A_ik[state] * A_jk[state];
        });
        A[state] /= (A ^ noarr::fix<'i'>(noarr::get_index<'j'>(state)))[state];
    });
    auto A_ii = A ^ noarr::fix<'j'>(noarr::get_index<'i'>(state));
    inner.order(noarr::slice<'k'>(0, noarr::get_index<'i'>(state))).template for_each<'k'>([=](auto state) {
        A_ii[state] -= A_ik[state] * A_ik[state];
    });
    A_ii[state] = std::sqrt(A_ii[state]);
});
y[noarr::idx<'i'>(0)] = -r[noarr::idx<'i'>(0)];
beta = 1;
alpha = -r[noarr::idx<'i'>(0)];
noarr::traverser(r, y, r_k, y_k)
    .order(noarr::shift<'k'>(1))
    .template for_dims<'k'>([=, &alpha, &beta, &sum, z = z.get_ref()](auto inner) {
        auto state = inner.state();
        beta = (1 - alpha * alpha) * beta;
        sum = 0;
        auto traverser = inner.order(noarr::slice<'i'>(0, noarr::get_index<'k'>(state)));
        traverser.for_each([=, &sum](auto state) {
            auto [i, k] = noarr::get_indices<'i', 'k'>(state);
            sum += r[noarr::idx<'i'>(k - i - 1)] * y[state];
        });
        alpha = -(r_k[state] + sum) / beta;
        traverser.for_each([=, &alpha](auto state) {
            auto [i, k] = noarr::get_indices<'i', 'k'>(state);
            z[state] = y[state] + alpha * y[noarr::idx<'i'>(k - i - 1)];
        });
        traverser.for_each([=](auto state) { y[state] = z[state]; });
        y_k[state] = alpha;
    });
noarr::traverser(A_ij, R, Q).template for_dims<'k'>([=](auto inner) {
    auto state = inner.state();
    num_t norm = 0;
    inner.template for_each<'i'>([=, &norm](auto state) { norm += A[state] * A[state]; });
    auto R_diag = R ^ noarr::fix<'j'>(noarr::get_index<'k'>(state));
    R_diag[state] = std::sqrt(norm);
    inner.template for_each<'i'>([=](auto state) { Q[state] = A[state] / R_diag[state]; });
    inner.order(noarr::shift<'j'>(noarr::get_index<'k'>(state) + 1)).template for_dims<'j'>([=](auto inner) {
        auto state = inner.state();
        R[state] = 0;
        inner.for_each([=](auto state) { R[state] = R[state] + Q[state] * A_ij[state]; });
        inner.for_each([=](auto state) { A_ij[state] = A_ij[state] - Q[state] * R[state]; });
    });
});
noarr::traverser(A, b, x, y, A_ik, A_kj).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    inner.order(noarr::slice<'j'>(0, noarr::get_index<'i'>(state))).template for_dims<'j'>([=](auto inner) {
        auto state = inner.state();
        num_t w = A[state];
        inner.order(noarr::slice<'k'>(0, noarr::get_index<'j'>(state))).template for_each<'k'>([=, &w](auto state) {
            w -= A_ik[state] * A_kj[state];
        });
        A[state] = w / (A ^ noarr::fix<'i'>(noarr::get_index<'j'>(state)))[state];
    });
    inner.order(noarr::shift<'j'>(noarr::get_index<'i'>(state))).template for_dims<'j'>([=](auto inner) {
        auto state = inner.state();
        num_t w = A[state];
        inner.order(noarr::slice<'k'>(0, noarr::get_index<'i'>(state))).template for_each<'k'>([=, &w](auto state) {
            w -= A_ik[state] * A_kj[state];
        });
        A[state] = w;
    });
});
noarr::traverser(A, b, y).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    num_t w = b[state];
    inner.order(noarr::slice<'j'>(0, noarr::get_index<'i'>(state))).template for_each<'j'>([=, &w](auto state) {
        w -= A[state] * y[noarr::idx<'i'>(noarr::get_index<'j'>(state))];
    });
    y[state] = w;
});
noarr::traverser(A, x).order(noarr::reverse<'i'>()).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    num_t w = y[state];
    inner.order(noarr::shift<'j'>(noarr::get_index<'i'>(state) + 1)).template for_each<'j'>([=, &w](auto state) {
        w -= A[state] * x[noarr::idx<'i'>(noarr::get_index<'j'>(state))];
    });
    x[state] = w / A[state & noarr::idx<'j'>(noarr::get_index<'i'>(state))];
});
noarr::traverser(A, A_ik, A_kj).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    inner.order(noarr::slice<'j'>(0, noarr::get_index<'i'>(state))).template for_dims<'j'>([=](auto inner) {
        auto state = inner.state();
        inner.order(noarr::slice<'k'>(0, noarr::get_index<'j'>(state))).for_each([=](auto state) {
            A[state] -= A_ik[state] * A_kj[state];
        });
        A[state] /= (A ^ noarr::fix<'i'>(noarr::get_index<'j'>(state)))[state];
    });
    inner.order(noarr::shift<'j'>(noarr::get_index<'i'>(state)))
        .order(noarr::slice<'k'>(0, noarr::get_index<'i'>(state)))
        .order(order)
        .for_each([=](auto state) { A[state] -= A_ik[state] * A_kj[state]; });
});
noarr::traverser(L, x, b).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    x[state] = b[state];
    inner.order(noarr::slice<'j'>(0, noarr::get_index<'i'>(state))).for_each([=](auto state) {
        x[state] -= L[state] * x_j[state];
    });
    x[state] = x[state] / L[state & noarr::idx<'j'>(noarr::get_index<'i'>(state))];
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
noarr::traverser(imgIn, y1).template for_dims<'w'>([=](auto inner) {
    num_t ym1 = 0;
    num_t ym2 = 0;
    num_t xm1 = 0;
    inner.for_each([=, &ym1, &ym2, &xm1](auto state) {
        y1[state] = a1 * imgIn[state] + a2 * xm1 + b1 * ym1 + b2 * ym2;
        xm1 = imgIn[state];
        ym2 = ym1;
        ym1 = y1[state];
    });
});
noarr::traverser(imgIn, y2).template for_dims<'w'>([=](auto inner) {
    num_t yp1 = 0;
    num_t yp2 = 0;
    num_t xp1 = 0;
    num_t xp2 = 0;
    inner.order(noarr::reverse<'h'>()).for_each([=, &yp1, &yp2, &xp1, &xp2](auto state) {
        y2[state] = a3 * xp1 + a4 * xp2 + b1 * yp1 + b2 * yp2;
        xp2 = xp1;
        xp1 = imgIn[state];
        yp2 = yp1;
        yp1 = y2[state];
    });
});
noarr::traverser(y1, y2, imgOut).for_each([=](auto state) { imgOut[state] = c1 * (y1[state] + y2[state]); });
noarr::traverser(imgOut, y1).template for_dims<'h'>([=](auto inner) {
    num_t tm1 = 0;
    num_t ym1 = 0;
    num_t ym2 = 0;
    inner.for_each([=, &tm1, &ym1, &ym2](auto state) {
        y1[state] = a5 * imgOut[state] + a6 * tm1 + b1 * ym1 + b2 * ym2;
        tm1 = imgOut[state];
        ym2 = ym1;
        ym1 = y1[state];
    });
});
noarr::traverser(imgOut, y2).template for_dims<'h'>([=](auto inner) {
    num_t tp1 = 0;
    num_t tp2 = 0;
    num_t yp1 = 0;
    num_t yp2 = 0;
    inner.order(noarr::reverse<'w'>()).for_each([=, &tp1, &tp2, &yp1, &yp2](auto state) {
        y2[state] = a7 * tp1 + a8 * tp2 + b1 * yp1 + b2 * yp2;
        tp2 = tp1;
        tp1 = imgOut[state];
        yp2 = yp1;
        yp1 = y2[state];
    });
});
noarr::traverser(y1, y2, imgOut).for_each([=](auto state) { imgOut[state] = c2 * (y1[state] + y2[state]); });
noarr::traverser(path, path_start_k, path_end_k).order(noarr::hoist<'k'>()).order(order).for_each([=](auto state) {
    path[state] = std::min(path_start_k[state] + path_end_k[state], path[state]);
});
noarr::traverser(seq, table, table_ik, table_kj).order(noarr::reverse<'i'>()).template for_dims<'i'>([=](auto inner) {
    auto state = inner.state();
    inner.order(noarr::shift<'j'>(noarr::get_index<'i'>(state) + 1)).template for_dims<'j'>([=](auto inner) {
        auto state = inner.state();
        if (noarr::get_index<'j'>(state) >= 0)
            table[state] = max_score(table[state], table[state - noarr::idx<'j'>(1)]);
        if (noarr::get_index<'i'>(state) + 1 < (table | noarr::get_length<'i'>()))
            table[state] = max_score(table[state], table[state + noarr::idx<'i'>(1)]);
        if (noarr::get_index<'j'>(state) >= 0 ||
            noarr::get_index<'i'>(state) + 1 < (table | noarr::get_length<'i'>())) {
            if (noarr::get_index<'i'>(state) < noarr::get_index<'j'>(state) - 1)
                table[state] = max_score(table[state], (table[state + noarr::idx<'i'>(1) - noarr::idx<'j'>(1)]) +
                                                           match(seq[state], seq_j[state]));
            else
                table[state] = max_score(table[state], (table[state + noarr::idx<'i'>(1) - noarr::idx<'j'>(1)]));
        }
        inner.order(noarr::span<'k'>(noarr::get_index<'i'>(state) + 1, noarr::get_index<'j'>(state)))
            .template for_each<'k'>([=](auto state) {
                table[state] = max_score(table[state], table_ik[state] + table_kj[state + noarr::idx<'k'>(1)]);
            });
    });
});

num_t DX = (num_t)1.0 / (traverser.top_struct() | noarr::get_length<'i'>());
num_t DY = (num_t)1.0 / (traverser.top_struct() | noarr::get_length<'j'>());
num_t DT = (num_t)1.0 / (traverser.top_struct() | noarr::get_length<'t'>());
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
traverser.order(noarr::symmetric_spans<'i', 'j'>(traverser.top_struct(), 1, 1)).template for_dims<'t'>([=](auto inner) {
    inner.template for_dims<'i'>([=](auto inner) {
        auto state = inner.state();
        v[state & noarr::idx<'j'>(0)] = (num_t)1.0;
        p[state & noarr::idx<'j'>(0)] = (num_t)0.0;
        q[state & noarr::idx<'j'>(0)] = v[state & noarr::idx<'j'>(0)];
        inner.for_each([=](auto state) {
            p[state] = -c / (a * p[state - noarr::idx<'j'>(1)] + b);
            q[state] = (-d * u_trans[state - noarr::idx<'i'>(1)] + (B2 + B1 * d) * u_trans[state] -
                        f * u_trans[state + noarr::idx<'i'>(1)] - a * q[state - noarr::idx<'j'>(1)]) /
                       (a * p[state - noarr::idx<'j'>(1)] + b);
        });
        v[state & noarr::idx<'j'>((traverser.top_struct() | noarr::get_length<'j'>()) - 1)] = (num_t)1.0;
        inner.order(noarr::reverse<'j'>()).for_each([=](auto state) {
            v[state] = p[state] * v[state + noarr::idx<'j'>(1)] + q[state];
        });
    });
    inner.template for_dims<'i'>([=](auto inner) {
        auto state = inner.state();
        u[state & noarr::idx<'j'>(0)] = (num_t)1.0;
        p[state & noarr::idx<'j'>(0)] = (num_t)0.0;
        q[state & noarr::idx<'j'>(0)] = u[state & noarr::idx<'j'>(0)];
        inner.for_each([=](auto state) {
            p[state] = -f / (d * p[state - noarr::idx<'j'>(1)] + e);
            q[state] = (-a * v_trans[state - noarr::idx<'i'>(1)] + (B2 + B1 * a) * v_trans[state] -
                        c * v_trans[state + noarr::idx<'i'>(1)] - d * q[state - noarr::idx<'j'>(1)]) /
                       (d * p[state - noarr::idx<'j'>(1)] + e);
        });
        u[state & noarr::idx<'j'>((traverser.top_struct() | noarr::get_length<'j'>()) - 1)] = (num_t)1.0;
        inner.order(noarr::reverse<'j'>()).for_each([=](auto state) {
            u[state] = p[state] * u[state + noarr::idx<'j'>(1)] + q[state];
        });
    });
});
noarr::traverser(ex, ey, hz, _fict_).template for_dims<'t'>([=](auto inner) {
    inner.order(noarr::shift<'i'>(1)).template for_each<'j'>([=](auto state) {
        ey[state & noarr::idx<'i'>(0)] = _fict_[state];
    });
    inner.order(noarr::shift<'i'>(1)).for_each([=](auto state) {
        ey[state] = ey[state] - (num_t).5 * (hz[state] - hz[state - noarr::idx<'i'>(1)]);
    });
    inner.order(noarr::shift<'j'>(1)).for_each([=](auto state) {
        ex[state] = ex[state] - (num_t).5 * (hz[state] - hz[state - noarr::idx<'j'>(1)]);
    });
    inner
        .order(noarr::span<'i'>(0, (inner.top_struct() | noarr::get_length<'i'>()) - 1) ^
               noarr::span<'j'>(0, (inner.top_struct() | noarr::get_length<'j'>()) - 1))
        .for_each([=](auto state) {
            hz[state] = hz[state] - (num_t).7 * (ex[state + noarr::idx<'j'>(1)] - ex[state] +
                                                 ey[state + noarr::idx<'i'>(1)] - ey[state]);
        });
});
traverser.order(noarr::symmetric_spans<'i', 'j', 'k'>(traverser.top_struct(), 1, 1, 1))
    .order(order)
    .template for_dims<'t'>([=](auto inner) {
        inner.for_each([=](auto state) {
            B[state] = (num_t).125 * (A[state - noarr::idx<'i'>(1)] - 2 * A[state] + A[state + noarr::idx<'i'>(1)]) +
                       (num_t).125 * (A[state - noarr::idx<'j'>(1)] - 2 * A[state] + A[state + noarr::idx<'j'>(1)]) +
                       (num_t).125 * (A[state - noarr::idx<'k'>(1)] - 2 * A[state] + A[state + noarr::idx<'k'>(1)]) +
                       A[state];
        });
        inner.for_each([=](auto state) {
            A[state] = (num_t).125 * (B[state - noarr::idx<'i'>(1)] - 2 * B[state] + B[state + noarr::idx<'i'>(1)]) +
                       (num_t).125 * (B[state - noarr::idx<'j'>(1)] - 2 * B[state] + B[state + noarr::idx<'j'>(1)]) +
                       (num_t).125 * (B[state - noarr::idx<'k'>(1)] - 2 * B[state] + B[state + noarr::idx<'k'>(1)]) +
                       B[state];
        });
    });
traverser.template for_dims<'t'>([=](auto inner) {
    inner.order(noarr::symmetric_span<'i'>(traverser.top_struct(), 1)).for_each([=](auto state) {
        B[state] = 0.33333 * (A[state - noarr::idx<'i'>(1)] + A[state] + A[state + noarr::idx<'i'>(1)]);
    });
    inner.order(noarr::symmetric_span<'i'>(traverser.top_struct(), 1)).for_each([=](auto state) {
        A[state] = 0.33333 * (B[state - noarr::idx<'i'>(1)] + B[state] + B[state + noarr::idx<'i'>(1)]);
    });
});
traverser.order(noarr::symmetric_spans<'i', 'j'>(traverser.top_struct(), 1, 1))
    .order(order)
    .template for_dims<'t'>([=](auto inner) {
        inner.for_each([=](auto state) {
            B[state] = (num_t).2 * (A[state] + A[state - noarr::idx<'j'>(1)] + A[state + noarr::idx<'j'>(1)] +
                                    A[state + noarr::idx<'i'>(1)] + A[state - noarr::idx<'i'>(1)]);
        });
        inner.for_each([=](auto state) {
            A[state] = (num_t).2 * (B[state] + B[state - noarr::idx<'j'>(1)] + B[state + noarr::idx<'j'>(1)] +
                                    B[state + noarr::idx<'i'>(1)] + B[state - noarr::idx<'i'>(1)]);
        });
    });
traverser.order(noarr::symmetric_spans<'i', 'j'>(traverser.top_struct(), 1, 1))
    .order(noarr::reorder<'t', 'i', 'j'>())
    .for_each([=](auto state) {
        A[state] = (A[state - noarr::idx<'i'>(1) - noarr::idx<'j'>(1)] + A[state - noarr::idx<'i'>(1)] +
                    A[state - noarr::idx<'i'>(1) + noarr::idx<'j'>(1)] + A[state - noarr::idx<'j'>(1)] + A[state] +
                    A[state + noarr::idx<'j'>(1)] + A[state + noarr::idx<'i'>(1) - noarr::idx<'j'>(1)] + A[state + noarr::idx<'i'>(1)] +
                    A[state + noarr::idx<'i'>(1) + noarr::idx<'j'>(1)]) /
                   (num_t)9.0;
    });
