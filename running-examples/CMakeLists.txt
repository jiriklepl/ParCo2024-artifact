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
  set(NOARR_STRUCTURES_BRANCH PMAM2024)
endif()

# download Noarr
FetchContent_Declare(
  Noarr
  GIT_REPOSITORY https://github.com/jiriklepl/noarr-structures.git
  GIT_TAG        ${NOARR_STRUCTURES_BRANCH})
FetchContent_MakeAvailable(Noarr)

include_directories(${Noarr_SOURCE_DIR}/include)

if (DEFINED ENV{TBBROOT})
  link_directories($ENV{TBBROOT}/lib)
endif ()

add_executable(matmul matmul.cpp)
add_executable(matmul-blocked matmul.cpp)
target_compile_definitions(matmul-blocked PRIVATE MATMUL_BLOCKED=1)
add_executable(matmul-factored matmul_factored.cpp)
add_executable(histogram-cu histogram.cu)
add_executable(histogram-cpp histogram.cpp)
target_compile_definitions(histogram-cpp PRIVATE HISTO_IMPL=histo_foreach)
add_executable(histogram-cpp-parallel histogram.cpp)
target_compile_definitions(histogram-cpp-parallel PRIVATE HISTO_IMPL=histo_tbbreduce HISTO_HAVE_TBB=1)
target_link_libraries(histogram-cpp-parallel tbb)
