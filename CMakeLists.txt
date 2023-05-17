cmake_minimum_required(VERSION 3.0.0)
project(mpi-openmp VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(include)
add_executable(sequential-comp sequential_comparison.cpp)
add_executable(mpi-openmp mpi-openmp.cpp src/VectorDistribution.cpp include/VectorDistribution.hpp)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(C_MAKE_CP)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fopenmp")