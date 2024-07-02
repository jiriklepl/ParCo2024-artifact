cmake_minimum_required(VERSION 3.10)
Include(FetchContent)

# set the project name and version
project(
  NoarrPolybenchOMP VERSION 4.2.1
  DESCRIPTION "Polybench kernels implemented using Noarr Structures - openmp parallelized"
  LANGUAGES CXX
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT DEFINED NOARR_STRUCTURES_BRANCH)
  set(NOARR_STRUCTURES_BRANCH multimerge)
endif()

# download Noarr Structures
FetchContent_Declare(
  Noarr
  GIT_REPOSITORY https://github.com/jiriklepl/noarr-structures.git
  GIT_TAG        ${NOARR_STRUCTURES_BRANCH})
FetchContent_MakeAvailable(Noarr)

include_directories(include)
include_directories(${Noarr_SOURCE_DIR}/include)

add_compile_options(-fopenmp -fopenmp-simd)

if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# datamining
add_executable(covariance datamining/covariance/covariance.cpp)
target_link_libraries(covariance -fopenmp)

# linear algebra - blas
add_executable(syr2k linear-algebra/blas/syr2k/syr2k.cpp)
target_link_libraries(syr2k -fopenmp)

# linear algebra - kernels
add_executable(2mm linear-algebra/kernels/2mm/2mm.cpp)
target_link_libraries(2mm -fopenmp)

# stencils
add_executable(heat-3d stencils/heat-3d/heat-3d.cpp)
target_link_libraries(heat-3d -fopenmp)
