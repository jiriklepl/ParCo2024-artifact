cmake_minimum_required(VERSION 3.10)

# set the project name
project(PolybenchC VERSION 4.2.1 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

link_libraries(m)
include_directories(utilities)

add_executable(covariance datamining/covariance/covariance.c utilities/polybench.c)
add_executable(gemm linear-algebra/blas/gemm/gemm.c utilities/polybench.c)
add_executable(2mm linear-algebra/kernels/2mm/2mm.c utilities/polybench.c)
add_executable(floyd-warshall medley/floyd-warshall/floyd-warshall.c utilities/polybench.c)
