cmake_minimum_required(VERSION 3.5)
project(miniSqlFrom0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    BM/bm_hashTable.cpp BM/bm_hashTable.h BM/bm_buffermgr.cpp BM/bm_buffermgr.h PF/pf_filemgr.cpp  PF/pf_exception.h RM/rm.cpp RM/rm.h RM/rm_fileHandler.cpp RM/rm_pageHandler.cpp
        RM/rm_fileScan.cpp IX/ix.h IX/IX_manager.cpp IX/ix_IndexHandler.cpp IX/IX_pageHandler.cpp IX/ix_test.cpp IX/ix_Key.cpp IX/ix_IndexScan.cpp)


add_executable(miniSqlFrom0 ${SOURCE_FILES})