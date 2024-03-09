cmake_minimum_required(VERSION 3.10)
Include(FetchContent)

# set the project name and version
project(
  NoarrPolybench VERSION 0.0.1
  DESCRIPTION "Polybench kernels implemented using Noarr Structures"
  LANGUAGES CXX
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT DEFINED NOARR_STRUCTURES_BRANCH)
  set(NOARR_STRUCTURES_BRANCH main)
endif()

# download Noarr
FetchContent_Declare(
  Noarr
  GIT_REPOSITORY https://github.com/jiriklepl/noarr-structures.git
  GIT_TAG        ${NOARR_STRUCTURES_BRANCH})
FetchContent_MakeAvailable(Noarr)

include_directories(include)
include_directories(${Noarr_SOURCE_DIR}/include)

if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

if (DEFINED ENV{TBBROOT})
  link_directories($ENV{TBBROOT}/lib)
endif ()

# datamining
add_executable(covariance datamining/covariance/covariance.cpp)
target_link_libraries(covariance tbb)

# linear algebra - blas
add_executable(syr2k linear-algebra/blas/syr2k/syr2k.cpp)
target_link_libraries(syr2k tbb)

# linear algebra - kernels
add_executable(2mm linear-algebra/kernels/2mm/2mm.cpp)
target_link_libraries(2mm tbb)

# stencils
add_executable(heat-3d stencils/heat-3d/heat-3d.cpp)
target_link_libraries(heat-3d tbb)
