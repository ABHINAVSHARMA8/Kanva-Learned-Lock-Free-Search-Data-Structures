cmake_minimum_required(VERSION 3.2)
project(xxx)
ADD_DEFINITIONS(-std=c++17)

set(CMAKE_CXX_STANDARD 17)



find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include_directories("include")
include_directories("/opt/intel/oneapi/mkl/2021.1.1/include")

set(CMAKE_CXX_FLAGS "-g -msse -march=core-avx2")



file(GLOB finedex_benchmark_SORUCES ""  "finedex_benchmark.cpp" )

add_executable(finedex_benchmark  ${finedex_benchmark_SORUCES} )
target_link_libraries(finedex_benchmark pthread)
