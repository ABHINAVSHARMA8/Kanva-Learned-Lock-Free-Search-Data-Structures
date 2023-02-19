cmake_minimum_required(VERSION 3.2)
project(xxx)
ADD_DEFINITIONS(-std=c++17)

include_directories("include")
include_directories("/opt/intel/oneapi/mkl/2021.1.1/include")

set(CMAKE_CXX_FLAGS "-g -msse -march=core-avx2")



file(GLOB finedex_benchmark_SORUCES ""  "finedex_benchmark.cpp" )
file(GLOB PLEX_INCLUDE_H "include/ts/*.h")
file(GLOB PLEX_INCLUDE_H "include/rs/*.h")
add_executable(finedex_benchmark ${PLEX_INCLUDE_H} ${finedex_benchmark_SORUCES} )
target_link_libraries(finedex_benchmark pthread)