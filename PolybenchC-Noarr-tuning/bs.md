# Benchmark algorithm analysiS

## Creates temporary variables dependent on traversal order or caches such values into arrays

**These require a slight reimplementation to support tiling:**

- atax
- covariance (upper triangular)
  - consists of multiple independent traversals
  - the penultimate traversal supports tiling of `i` and `k`, but not `j`
- gesummv
- gemm
- syr2k (triangular)
  - supports tiling of `i` and `k`, but not `j`
- syrk (triangular)
  - supports tiling of `i` and `k`, but not `j`

**These cannot be easily reimplemented to support tiling:**

- deriche
- durbin (triangular)
- gramschmidt
- trisolv (triangular)

## Data dependent on traversal order

*Transformations of these are currently not supported by Noarr.*

- jacobi-1d - there is nothing to do

- adi
- bicg
- fdtd-2d

- cholesky (triangular)
- lu (triangular)
- ludcmp (triangular)
- nussinov (upper triangular)
- correlation (upper triangular)

- seidel-2d - this one is *hardcore*

## No dependencies between (some) traversal dimensions

These are the easiest to transform, some of them do not even require the planner.

**These allow full transformation:**

- floyd-warshall
- gemver
- mvt

**These allow partial transformation:**

- 2mm
  - can be reimplemented to allow higher transformation
- 3mm
  - can be reimplemented to allow higher transformation
- doitgen
- heat-3d
- jacobi-2d

- symm (triangular)
- trmm (upper triangular)
