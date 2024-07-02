cmake_minimum_required(VERSION 3.10)
Include(FetchContent)

# set the project name and version
project(
  NoarrPolybenchTuning VERSION 4.2.1
  DESCRIPTION "Polybench kernels implemented using Noarr Structures - Tuning"
  LANGUAGES CXX
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT DEFINED NOARR_STRUCTURES_BRANCH)
  set(NOARR_STRUCTURES_BRANCH multimerge)
endif()

if(NOT DEFINED NOARR_TUNING_BRANCH)
  set(NOARR_TUNING_BRANCH ParCo2024)
endif()

# download Noarr Structures
FetchContent_Declare(
  Noarr
  GIT_REPOSITORY https://github.com/jiriklepl/noarr-structures.git
  GIT_TAG        ${NOARR_STRUCTURES_BRANCH})
FetchContent_MakeAvailable(Noarr)

# download Noarr Tuning
FetchContent_Declare(
  NoarrTuning
  GIT_REPOSITORY https://github.com/jiriklepl/noarr-tuning.git
  GIT_TAG        ${NOARR_TUNING_BRANCH})
FetchContent_MakeAvailable(NoarrTuning)

include_directories(include)
include_directories(${Noarr_SOURCE_DIR}/include)
include_directories(${NoarrTuning_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(autotune)

macro(add_tuning_target target source)
  add_executable(${target} ${source})
  target_precompile_headers(${target} PRIVATE <chrono> <cmath> <iomanip> <iostream>)
  target_precompile_headers(${target} PRIVATE ${Noarr_SOURCE_DIR}/include/noarr/traversers.hpp)
  target_precompile_headers(${target} PRIVATE include/defines.hpp include/utility.hpp)
  target_precompile_headers(${target} PRIVATE ${NoarrTuning_SOURCE_DIR}/include/noarr/tuning/formatters/opentuner_formatter.hpp)
  target_compile_definitions(${target} PRIVATE NOARR_TUNING_DEFINES_FILE=defines.ipp)

  add_executable(${target}_autotune ${source})
  target_compile_definitions(${target}_autotune PRIVATE NOARR_TUNE NOARR_TUNING_DEFINES_FILE=defines.ipp)

  add_dependencies(autotune ${target}_autotune)

  add_executable(${target}_autotuned ${source})
endmacro()

file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/defines.ipp)

# datamining
add_tuning_target(covariance datamining/covariance/covariance.cpp)

# linear algebra - blas
add_tuning_target(gemm linear-algebra/blas/gemm/gemm.cpp)
add_tuning_target(gemver linear-algebra/blas/gemver/gemver.cpp)
add_tuning_target(gesummv linear-algebra/blas/gesummv/gesummv.cpp)
add_tuning_target(symm linear-algebra/blas/symm/symm.cpp)
add_tuning_target(syr2k linear-algebra/blas/syr2k/syr2k.cpp)
add_tuning_target(syrk linear-algebra/blas/syrk/syrk.cpp)
add_tuning_target(trmm linear-algebra/blas/trmm/trmm.cpp)

# linear algebra - kernels
add_tuning_target(2mm linear-algebra/kernels/2mm/2mm.cpp)
add_tuning_target(3mm linear-algebra/kernels/3mm/3mm.cpp)
add_tuning_target(atax linear-algebra/kernels/atax/atax.cpp)
add_tuning_target(bicg linear-algebra/kernels/bicg/bicg.cpp)
add_tuning_target(doitgen linear-algebra/kernels/doitgen/doitgen.cpp)
add_tuning_target(mvt linear-algebra/kernels/mvt/mvt.cpp)

# linear algebra - solvers
add_tuning_target(cholesky linear-algebra/solvers/cholesky/cholesky.cpp)
add_tuning_target(gramschmidt linear-algebra/solvers/gramschmidt/gramschmidt.cpp)
add_tuning_target(lu linear-algebra/solvers/lu/lu.cpp)
add_tuning_target(ludcmp linear-algebra/solvers/ludcmp/ludcmp.cpp)
add_tuning_target(trisolv linear-algebra/solvers/trisolv/trisolv.cpp)

# medley
add_tuning_target(deriche medley/deriche/deriche.cpp)
add_tuning_target(floyd-warshall medley/floyd-warshall/floyd-warshall.cpp)
add_tuning_target(nussinov medley/nussinov/nussinov.cpp)

# stencils
add_tuning_target(adi stencils/adi/adi.cpp)
add_tuning_target(fdtd-2d stencils/fdtd-2d/fdtd-2d.cpp)
add_tuning_target(heat-3d stencils/heat-3d/heat-3d.cpp)
add_tuning_target(jacobi-2d stencils/jacobi-2d/jacobi-2d.cpp)
add_tuning_target(seidel-2d stencils/seidel-2d/seidel-2d.cpp)
