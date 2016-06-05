cmake_minimum_required(VERSION 3.5)
project(miniSqlFrom0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    BM/bm_hashTable.cpp BM/bm_error.h BM/bm_hashTable_test.cpp BM/bm_hashTable.h)

add_executable(miniSqlFrom0 ${SOURCE_FILES})