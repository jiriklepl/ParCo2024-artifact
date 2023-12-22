cmake_minimum_required(VERSION 3.10)

# set the project name
project(PolybenchC VERSION 4.2.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

link_libraries(m)
include_directories(utilities)

if (DEFINED ENV{TBBROOT})
    link_directories($ENV{TBBROOT}/lib)
endif ()

# datamining
add_executable(covariance datamining/covariance/covariance.cpp utilities/polybench.c)
target_link_libraries(covariance tbb)

# linear-algebra - blas
add_executable(syr2k linear-algebra/blas/syr2k/syr2k.cpp utilities/polybench.c)
target_link_libraries(syr2k tbb)

# linear-algebra - kernels
add_executable(2mm linear-algebra/kernels/2mm/2mm.cpp utilities/polybench.c)
target_link_libraries(2mm tbb)

# stencils
add_executable(heat-3d stencils/heat-3d/heat-3d.cpp utilities/polybench.c)
target_link_libraries(heat-3d tbb)
