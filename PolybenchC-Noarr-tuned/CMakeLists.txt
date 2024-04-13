cmake_minimum_required(VERSION 3.10)
Include(FetchContent)

# set the project name and version
project(
  NoarrPolybenchTuned VERSION 4.2.1
  DESCRIPTION "Polybench kernels implemented using Noarr Structures - tuned"
  LANGUAGES CXX
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT DEFINED NOARR_STRUCTURES_BRANCH)
  set(NOARR_STRUCTURES_BRANCH main)
endif()

# download Noarr Structures
FetchContent_Declare(
  Noarr
  GIT_REPOSITORY https://github.com/jiriklepl/noarr-structures.git
  GIT_TAG        ${NOARR_STRUCTURES_BRANCH})
FetchContent_MakeAvailable(Noarr)

include_directories(include)
include_directories(${Noarr_SOURCE_DIR}/include)

# datamining
add_executable(covariance datamining/covariance/covariance.cpp)

# linear algebra - blas
add_executable(gemm linear-algebra/blas/gemm/gemm.cpp)

# linear algebra - kernels
add_executable(2mm linear-algebra/kernels/2mm/2mm.cpp)

# medley
add_executable(floyd-warshall medley/floyd-warshall/floyd-warshall.cpp)

if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()
