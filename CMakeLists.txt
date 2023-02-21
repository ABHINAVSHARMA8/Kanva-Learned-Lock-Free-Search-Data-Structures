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
file(GLOB finedex_search_SORUCES ""  "finedex_searchinsert.cpp" )
file(GLOB finedex_ycsb_SORUCES ""  "finedex_ycsb.cpp" )
file(GLOB PLEX_INCLUDE_H "include/plex/ts/*.h")
file(GLOB PLEX_INCLUDE_H "include/plex/rs/*.h")
add_executable(finedex_benchmark ${PLEX_INCLUDE_H} ${finedex_benchmark_SORUCES} )
add_executable(finedex_search ${PLEX_INCLUDE_H} ${finedex_search_SORUCES} )
add_executable(finedex_ycsb ${PLEX_INCLUDE_H} ${finedex_ycsb_SORUCES} )
target_link_libraries(finedex_benchmark pthread)