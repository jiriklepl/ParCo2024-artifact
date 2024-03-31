cmake_minimum_required(VERSION 3.10)

# set the project name
project(PolybenchC VERSION 4.2.1 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

link_libraries(m)
include_directories(utilities)

add_compile_options(-fopenmp -fopenmp-simd)
link_libraries(-lm -fopenmp)


# datamining
add_executable(covariance datamining/covariance/covariance.c utilities/polybench.c)


# linear-algebra - blas
add_executable(syr2k linear-algebra/blas/syr2k/syr2k.c utilities/polybench.c)

# linear-algebra - kernels
add_executable(2mm linear-algebra/kernels/2mm/2mm.c utilities/polybench.c)

# stencils
add_executable(heat-3d stencils/heat-3d/heat-3d.c utilities/polybench.c)
