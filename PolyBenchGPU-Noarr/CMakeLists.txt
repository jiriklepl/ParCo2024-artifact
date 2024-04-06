cmake_minimum_required(VERSION 3.10)
Include(FetchContent)

# set the project name
project(
  NoarrPolybenchGPU VERSION 0.0.1
  DESCRIPTION "NoarrPolybenchGPU is a GPU implementation of the Polybench benchmark suite."
  LANGUAGES CXX
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the CUDA standard
set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED True)

# set the CUDA flags
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -allow-unsupported-compiler --expt-relaxed-constexpr")

enable_language(CUDA)

if(NOT DEFINED NOARR_STRUCTURES_BRANCH)
  set(NOARR_STRUCTURES_BRANCH main)
endif()

# download the Noarr Structures library
FetchContent_Declare(
  Noarr
  GIT_REPOSITORY https://github.com/jiriklepl/noarr-structures.git
  GIT_TAG        ${NOARR_STRUCTURES_BRANCH})
FetchContent_MakeAvailable(Noarr)

include_directories(include)
include_directories(${Noarr_SOURCE_DIR}/include)

add_executable(runner
  CPP/main.cpp
  CUDA/GEMM/gemm.cu
  CUDA/2DCONV/2DConvolution.cu
  CUDA/2MM/2mm.cu
  CUDA/JACOBI2D/jacobi2d.cu
  CUDA/GRAMSCHM/gramschmidt.cu
)
