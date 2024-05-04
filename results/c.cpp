
for (j = 0; j < _PB_M; j++) {
    mean[j] = SCALAR_VAL(0.0);
    for (i = 0; i < _PB_N; i++)
        mean[j] += data[i][j];
    mean[j] /= float_n;
}
for (j = 0; j < _PB_M; j++) {
    stddev[j] = SCALAR_VAL(0.0);
    for (i = 0; i < _PB_N; i++)
        stddev[j] += (data[i][j] - mean[j]) * (data[i][j] - mean[j]);
    stddev[j] /= float_n;
    stddev[j] = SQRT_FUN(stddev[j]);
    stddev[j] = stddev[j] <= eps ? SCALAR_VAL(1.0) : stddev[j];
}
for (i = 0; i < _PB_N; i++)
    for (j = 0; j < _PB_M; j++) {
        data[i][j] -= mean[j];
        data[i][j] /= SQRT_FUN(float_n) * stddev[j];
    }
for (i = 0; i < _PB_M - 1; i++) {
    corr[i][i] = SCALAR_VAL(1.0);
    for (j = i + 1; j < _PB_M; j++) {
        corr[i][j] = SCALAR_VAL(0.0);
        for (k = 0; k < _PB_N; k++)
            corr[i][j] += (data[k][i] * data[k][j]);
        corr[j][i] = corr[i][j];
    }
}
corr[_PB_M - 1][_PB_M - 1] = SCALAR_VAL(1.0);
for (j = 0; j < _PB_M; j++) {
    mean[j] = SCALAR_VAL(0.0);
    for (i = 0; i < _PB_N; i++)
        mean[j] += data[i][j];
    mean[j] /= float_n;
}
for (i = 0; i < _PB_N; i++)
    for (j = 0; j < _PB_M; j++)
        data[i][j] -= mean[j];
for (i = 0; i < _PB_M; i++)
    for (j = i; j < _PB_M; j++) {
        cov[i][j] = SCALAR_VAL(0.0);
        for (k = 0; k < _PB_N; k++)
            cov[i][j] += data[k][i] * data[k][j];
        cov[i][j] /= (float_n - SCALAR_VAL(1.0));
        cov[j][i] = cov[i][j];
    }
for (i = 0; i < _PB_NI; i++) {
    for (j = 0; j < _PB_NJ; j++)
        C[i][j] *= beta;
    for (k = 0; k < _PB_NK; k++) {
        for (j = 0; j < _PB_NJ; j++)
            C[i][j] += alpha * A[i][k] * B[k][j];
    }
}

for (i = 0; i < _PB_N; i++)
    for (j = 0; j < _PB_N; j++)
        A[i][j] = A[i][j] + u1[i] * v1[j] + u2[i] * v2[j];
for (i = 0; i < _PB_N; i++)
    for (j = 0; j < _PB_N; j++)
        x[i] = x[i] + beta * A[j][i] * y[j];
for (i = 0; i < _PB_N; i++)
    x[i] = x[i] + z[i];
for (i = 0; i < _PB_N; i++)
    for (j = 0; j < _PB_N; j++)
        w[i] = w[i] + alpha * A[i][j] * x[j];
for (i = 0; i < _PB_N; i++) {
    tmp[i] = SCALAR_VAL(0.0);
    y[i] = SCALAR_VAL(0.0);
    for (j = 0; j < _PB_N; j++) {
        tmp[i] = A[i][j] * x[j] + tmp[i];
        y[i] = B[i][j] * x[j] + y[i];
    }
    y[i] = alpha * tmp[i] + beta * y[i];
}
for (i = 0; i < _PB_M; i++)
    for (j = 0; j < _PB_N; j++) {
        temp2 = 0;
        for (k = 0; k < i; k++) {
            C[k][j] += alpha * B[i][j] * A[i][k];
            temp2 += B[k][j] * A[i][k];
        }
        C[i][j] = beta * C[i][j] + alpha * B[i][j] * A[i][i] + alpha * temp2;
    }
for (i = 0; i < _PB_N; i++) {
    for (j = 0; j <= i; j++)
        C[i][j] *= beta;
    for (k = 0; k < _PB_M; k++)
        for (j = 0; j <= i; j++) {
            C[i][j] += A[j][k] * alpha * B[i][k] + B[j][k] * alpha * A[i][k];
        }
}
for (i = 0; i < _PB_N; i++) {
    for (j = 0; j <= i; j++)
        C[i][j] *= beta;
    for (k = 0; k < _PB_M; k++) {
        for (j = 0; j <= i; j++)
            C[i][j] += alpha * A[i][k] * A[j][k];
    }
}
for (i = 0; i < _PB_M; i++)
    for (j = 0; j < _PB_N; j++) {
        for (k = i + 1; k < _PB_M; k++)
            B[i][j] += A[k][i] * B[k][j];
        B[i][j] = alpha * B[i][j];
    }

for (i = 0; i < _PB_NI; i++)
    for (j = 0; j < _PB_NJ; j++) {
        tmp[i][j] = SCALAR_VAL(0.0);
        for (k = 0; k < _PB_NK; ++k)
            tmp[i][j] += alpha * A[i][k] * B[k][j];
    }
for (i = 0; i < _PB_NI; i++)
    for (j = 0; j < _PB_NL; j++) {
        D[i][j] *= beta;
        for (k = 0; k < _PB_NJ; ++k)
            D[i][j] += tmp[i][k] * C[k][j];
    }

for (i = 0; i < _PB_NI; i++)
    for (j = 0; j < _PB_NJ; j++) {
        E[i][j] = SCALAR_VAL(0.0);
        for (k = 0; k < _PB_NK; ++k)
            E[i][j] += A[i][k] * B[k][j];
    }
for (i = 0; i < _PB_NJ; i++)
    for (j = 0; j < _PB_NL; j++) {
        F[i][j] = SCALAR_VAL(0.0);
        for (k = 0; k < _PB_NM; ++k)
            F[i][j] += C[i][k] * D[k][j];
    }
for (i = 0; i < _PB_NI; i++)
    for (j = 0; j < _PB_NL; j++) {
        G[i][j] = SCALAR_VAL(0.0);
        for (k = 0; k < _PB_NJ; ++k)
            G[i][j] += E[i][k] * F[k][j];
    }
for (i = 0; i < _PB_N; i++)
    y[i] = 0;
for (i = 0; i < _PB_M; i++) {
    tmp[i] = SCALAR_VAL(0.0);
    for (j = 0; j < _PB_N; j++)
        tmp[i] = tmp[i] + A[i][j] * x[j];
    for (j = 0; j < _PB_N; j++)
        y[j] = y[j] + A[i][j] * tmp[i];
}
for (i = 0; i < _PB_M; i++)
    s[i] = 0;
for (i = 0; i < _PB_N; i++) {
    q[i] = SCALAR_VAL(0.0);
    for (j = 0; j < _PB_M; j++) {
        s[j] = s[j] + r[i] * A[i][j];
        q[i] = q[i] + A[i][j] * p[j];
    }
}
for (r = 0; r < _PB_NR; r++)
    for (q = 0; q < _PB_NQ; q++) {
        for (p = 0; p < _PB_NP; p++) {
            sum[p] = SCALAR_VAL(0.0);
            for (s = 0; s < _PB_NP; s++)
                sum[p] += A[r][q][s] * C4[s][p];
        }
        for (p = 0; p < _PB_NP; p++)
            A[r][q][p] = sum[p];
    }
for (i = 0; i < _PB_N; i++)
    for (j = 0; j < _PB_N; j++)
        x1[i] = x1[i] + A[i][j] * y_1[j];
for (i = 0; i < _PB_N; i++)
    for (j = 0; j < _PB_N; j++)
        x2[i] = x2[i] + A[j][i] * y_2[j];
for (i = 0; i < _PB_N; i++) {
    for (j = 0; j < i; j++) {
        for (k = 0; k < j; k++) {
            A[i][j] -= A[i][k] * A[j][k];
        }
        A[i][j] /= A[j][j];
    }
    for (k = 0; k < i; k++) {
        A[i][i] -= A[i][k] * A[i][k];
    }
    A[i][i] = SQRT_FUN(A[i][i]);
}
y[0] = -r[0];
beta = SCALAR_VAL(1.0);
alpha = -r[0];
for (k = 1; k < _PB_N; k++) {
    beta = (1 - alpha * alpha) * beta;
    sum = SCALAR_VAL(0.0);
    for (i = 0; i < k; i++) {
        sum += r[k - i - 1] * y[i];
    }
    alpha = -(r[k] + sum) / beta;
    for (i = 0; i < k; i++) {
        z[i] = y[i] + alpha * y[k - i - 1];
    }
    for (i = 0; i < k; i++) {
        y[i] = z[i];
    }
    y[k] = alpha;
}
for (k = 0; k < _PB_N; k++) {
    nrm = SCALAR_VAL(0.0);
    for (i = 0; i < _PB_M; i++)
        nrm += A[i][k] * A[i][k];
    R[k][k] = SQRT_FUN(nrm);
    for (i = 0; i < _PB_M; i++)
        Q[i][k] = A[i][k] / R[k][k];
    for (j = k + 1; j < _PB_N; j++) {
        R[k][j] = SCALAR_VAL(0.0);
        for (i = 0; i < _PB_M; i++)
            R[k][j] += Q[i][k] * A[i][j];
        for (i = 0; i < _PB_M; i++)
            A[i][j] = A[i][j] - Q[i][k] * R[k][j];
    }
}
for (i = 0; i < _PB_N; i++) {
    for (j = 0; j < i; j++) {
        w = A[i][j];
        for (k = 0; k < j; k++) {
            w -= A[i][k] * A[k][j];
        }
        A[i][j] = w / A[j][j];
    }
    for (j = i; j < _PB_N; j++) {
        w = A[i][j];
        for (k = 0; k < i; k++) {
            w -= A[i][k] * A[k][j];
        }
        A[i][j] = w;
    }
}
for (i = 0; i < _PB_N; i++) {
    w = b[i];
    for (j = 0; j < i; j++)
        w -= A[i][j] * y[j];
    y[i] = w;
}
for (i = _PB_N - 1; i >= 0; i--) {
    w = y[i];
    for (j = i + 1; j < _PB_N; j++)
        w -= A[i][j] * x[j];
    x[i] = w / A[i][i];
}
for (i = 0; i < _PB_N; i++) {
    for (j = 0; j < i; j++) {
        for (k = 0; k < j; k++) {
            A[i][j] -= A[i][k] * A[k][j];
        }
        A[i][j] /= A[j][j];
    }
    for (j = i; j < _PB_N; j++) {
        for (k = 0; k < i; k++) {
            A[i][j] -= A[i][k] * A[k][j];
        }
    }
}
for (i = 0; i < _PB_N; i++) {
    x[i] = b[i];
    for (j = 0; j < i; j++)
        x[i] -= L[i][j] * x[j];
    x[i] = x[i] / L[i][i];
}
k = (SCALAR_VAL(1.0) - EXP_FUN(-alpha)) * (SCALAR_VAL(1.0) - EXP_FUN(-alpha)) /
    (SCALAR_VAL(1.0) + SCALAR_VAL(2.0) * alpha * EXP_FUN(-alpha) - EXP_FUN(SCALAR_VAL(2.0) * alpha));
a1 = a5 = k;
a2 = a6 = k * EXP_FUN(-alpha) * (alpha - SCALAR_VAL(1.0));
a3 = a7 = k * EXP_FUN(-alpha) * (alpha + SCALAR_VAL(1.0));
a4 = a8 = -k * EXP_FUN(SCALAR_VAL(-2.0) * alpha);
b1 = POW_FUN(SCALAR_VAL(2.0), -alpha);
b2 = -EXP_FUN(SCALAR_VAL(-2.0) * alpha);
c1 = c2 = 1;
for (i = 0; i < _PB_W; i++) {
    ym1 = SCALAR_VAL(0.0);
    ym2 = SCALAR_VAL(0.0);
    xm1 = SCALAR_VAL(0.0);
    for (j = 0; j < _PB_H; j++) {
        y1[i][j] = a1 * imgIn[i][j] + a2 * xm1 + b1 * ym1 + b2 * ym2;
        xm1 = imgIn[i][j];
        ym2 = ym1;
        ym1 = y1[i][j];
    }
}
for (i = 0; i < _PB_W; i++) {
    yp1 = SCALAR_VAL(0.0);
    yp2 = SCALAR_VAL(0.0);
    xp1 = SCALAR_VAL(0.0);
    xp2 = SCALAR_VAL(0.0);
    for (j = _PB_H - 1; j >= 0; j--) {
        y2[i][j] = a3 * xp1 + a4 * xp2 + b1 * yp1 + b2 * yp2;
        xp2 = xp1;
        xp1 = imgIn[i][j];
        yp2 = yp1;
        yp1 = y2[i][j];
    }
}
for (i = 0; i < _PB_W; i++)
    for (j = 0; j < _PB_H; j++) {
        imgOut[i][j] = c1 * (y1[i][j] + y2[i][j]);
    }
for (j = 0; j < _PB_H; j++) {
    tm1 = SCALAR_VAL(0.0);
    ym1 = SCALAR_VAL(0.0);
    ym2 = SCALAR_VAL(0.0);
    for (i = 0; i < _PB_W; i++) {
        y1[i][j] = a5 * imgOut[i][j] + a6 * tm1 + b1 * ym1 + b2 * ym2;
        tm1 = imgOut[i][j];
        ym2 = ym1;
        ym1 = y1[i][j];
    }
}
for (j = 0; j < _PB_H; j++) {
    tp1 = SCALAR_VAL(0.0);
    tp2 = SCALAR_VAL(0.0);
    yp1 = SCALAR_VAL(0.0);
    yp2 = SCALAR_VAL(0.0);
    for (i = _PB_W - 1; i >= 0; i--) {
        y2[i][j] = a7 * tp1 + a8 * tp2 + b1 * yp1 + b2 * yp2;
        tp2 = tp1;
        tp1 = imgOut[i][j];
        yp2 = yp1;
        yp1 = y2[i][j];
    }
}
for (i = 0; i < _PB_W; i++)
    for (j = 0; j < _PB_H; j++)
        imgOut[i][j] = c2 * (y1[i][j] + y2[i][j]);
for (k = 0; k < _PB_N; k++) {
    for (i = 0; i < _PB_N; i++)
        for (j = 0; j < _PB_N; j++)
            path[i][j] = path[i][j] < path[i][k] + path[k][j] ? path[i][j] : path[i][k] + path[k][j];
}
for (i = _PB_N - 1; i >= 0; i--) {
    for (j = i + 1; j < _PB_N; j++) {
        if (j - 1 >= 0)
            table[i][j] = max_score(table[i][j], table[i][j - 1]);
        if (i + 1 < _PB_N)
            table[i][j] = max_score(table[i][j], table[i + 1][j]);
        if (j - 1 >= 0 && i + 1 < _PB_N) {
            if (i < j - 1)
                table[i][j] = max_score(table[i][j], table[i + 1][j - 1] + match(seq[i], seq[j]));
            else
                table[i][j] = max_score(table[i][j], table[i + 1][j - 1]);
        }
        for (k = i + 1; k < j; k++) {
            table[i][j] = max_score(table[i][j], table[i][k] + table[k + 1][j]);
        }
    }
}

DX = SCALAR_VAL(1.0) / (DATA_TYPE)_PB_N;
DY = SCALAR_VAL(1.0) / (DATA_TYPE)_PB_N;
DT = SCALAR_VAL(1.0) / (DATA_TYPE)_PB_TSTEPS;
B1 = SCALAR_VAL(2.0);
B2 = SCALAR_VAL(1.0);
mul1 = B1 * DT / (DX * DX);
mul2 = B2 * DT / (DY * DY);
a = -mul1 / SCALAR_VAL(2.0);
b = SCALAR_VAL(1.0) + mul1;
c = a;
d = -mul2 / SCALAR_VAL(2.0);
e = SCALAR_VAL(1.0) + mul2;
f = d;
for (t = 1; t <= _PB_TSTEPS; t++) {
    for (i = 1; i < _PB_N - 1; i++) {
        v[0][i] = SCALAR_VAL(1.0);
        p[i][0] = SCALAR_VAL(0.0);
        q[i][0] = v[0][i];
        for (j = 1; j < _PB_N - 1; j++) {
            p[i][j] = -c / (a * p[i][j - 1] + b);
            q[i][j] = (-d * u[j][i - 1] + (SCALAR_VAL(1.0) + SCALAR_VAL(2.0) * d) * u[j][i] - f * u[j][i + 1] -
                       a * q[i][j - 1]) /
                      (a * p[i][j - 1] + b);
        }
        v[_PB_N - 1][i] = SCALAR_VAL(1.0);
        for (j = _PB_N - 2; j >= 1; j--) {
            v[j][i] = p[i][j] * v[j + 1][i] + q[i][j];
        }
    }
    for (i = 1; i < _PB_N - 1; i++) {
        u[i][0] = SCALAR_VAL(1.0);
        p[i][0] = SCALAR_VAL(0.0);
        q[i][0] = u[i][0];
        for (j = 1; j < _PB_N - 1; j++) {
            p[i][j] = -f / (d * p[i][j - 1] + e);
            q[i][j] = (-a * v[i - 1][j] + (SCALAR_VAL(1.0) + SCALAR_VAL(2.0) * a) * v[i][j] - c * v[i + 1][j] -
                       d * q[i][j - 1]) /
                      (d * p[i][j - 1] + e);
        }
        u[i][_PB_N - 1] = SCALAR_VAL(1.0);
        for (j = _PB_N - 2; j >= 1; j--) {
            u[i][j] = p[i][j] * u[i][j + 1] + q[i][j];
        }
    }
}

for (t = 0; t < _PB_TMAX; t++) {
    for (j = 0; j < _PB_NY; j++)
        ey[0][j] = _fict_[t];
    for (i = 1; i < _PB_NX; i++)
        for (j = 0; j < _PB_NY; j++)
            ey[i][j] = ey[i][j] - SCALAR_VAL(0.5) * (hz[i][j] - hz[i - 1][j]);
    for (i = 0; i < _PB_NX; i++)
        for (j = 1; j < _PB_NY; j++)
            ex[i][j] = ex[i][j] - SCALAR_VAL(0.5) * (hz[i][j] - hz[i][j - 1]);
    for (i = 0; i < _PB_NX - 1; i++)
        for (j = 0; j < _PB_NY - 1; j++)
            hz[i][j] = hz[i][j] - SCALAR_VAL(0.7) * (ex[i][j + 1] - ex[i][j] + ey[i + 1][j] - ey[i][j]);
}
for (t = 1; t <= TSTEPS; t++) {
    for (i = 1; i < _PB_N - 1; i++) {
        for (j = 1; j < _PB_N - 1; j++) {
            for (k = 1; k < _PB_N - 1; k++) {
                B[i][j][k] = SCALAR_VAL(0.125) * (A[i + 1][j][k] - SCALAR_VAL(2.0) * A[i][j][k] + A[i - 1][j][k]) +
                             SCALAR_VAL(0.125) * (A[i][j + 1][k] - SCALAR_VAL(2.0) * A[i][j][k] + A[i][j - 1][k]) +
                             SCALAR_VAL(0.125) * (A[i][j][k + 1] - SCALAR_VAL(2.0) * A[i][j][k] + A[i][j][k - 1]) +
                             A[i][j][k];
            }
        }
    }
    for (i = 1; i < _PB_N - 1; i++) {
        for (j = 1; j < _PB_N - 1; j++) {
            for (k = 1; k < _PB_N - 1; k++) {
                A[i][j][k] = SCALAR_VAL(0.125) * (B[i + 1][j][k] - SCALAR_VAL(2.0) * B[i][j][k] + B[i - 1][j][k]) +
                             SCALAR_VAL(0.125) * (B[i][j + 1][k] - SCALAR_VAL(2.0) * B[i][j][k] + B[i][j - 1][k]) +
                             SCALAR_VAL(0.125) * (B[i][j][k + 1] - SCALAR_VAL(2.0) * B[i][j][k] + B[i][j][k - 1]) +
                             B[i][j][k];
            }
        }
    }
}
for (t = 0; t < _PB_TSTEPS; t++) {
    for (i = 1; i < _PB_N - 1; i++)
        B[i] = 0.33333 * (A[i - 1] + A[i] + A[i + 1]);
    for (i = 1; i < _PB_N - 1; i++)
        A[i] = 0.33333 * (B[i - 1] + B[i] + B[i + 1]);
}
for (t = 0; t < _PB_TSTEPS; t++) {
    for (i = 1; i < _PB_N - 1; i++)
        for (j = 1; j < _PB_N - 1; j++)
            B[i][j] = SCALAR_VAL(0.2) * (A[i][j] + A[i][j - 1] + A[i][1 + j] + A[1 + i][j] + A[i - 1][j]);
    for (i = 1; i < _PB_N - 1; i++)
        for (j = 1; j < _PB_N - 1; j++)
            A[i][j] = SCALAR_VAL(0.2) * (B[i][j] + B[i][j - 1] + B[i][1 + j] + B[1 + i][j] + B[i - 1][j]);
}
for (t = 0; t <= _PB_TSTEPS - 1; t++)
    for (i = 1; i <= _PB_N - 2; i++)
        for (j = 1; j <= _PB_N - 2; j++)
            A[i][j] = (A[i - 1][j - 1] + A[i - 1][j] + A[i - 1][j + 1] + A[i][j - 1] + A[i][j] + A[i][j + 1] +
                       A[i + 1][j - 1] + A[i + 1][j] + A[i + 1][j + 1]) /
                      SCALAR_VAL(9.0);
