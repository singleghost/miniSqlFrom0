cmake_minimum_required(VERSION 3.5)
project(miniSqlFrom0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        BM/bm_hashTable.cpp BM/bm_hashTable.h BM/bm_buffermgr.cpp BM/bm_buffermgr.h PF/pf_filemgr.cpp PF/pf_exception.h RM/rm.cpp RM/rm.h RM/rm_fileHandler.cpp RM/rm_pageHandler.cpp
        RM/rm_fileScan.cpp IX/ix.h IX/ix_manager.cpp IX/ix_IndexHandler.cpp IX/IX_pageHandler.cpp IX/ix_Key.cpp IX/ix_IndexScan.cpp SM/sm_manager.cpp SM/printer.h SM/printer.cpp QL/ql.cpp QL/ql_node.cpp QL/ql_relNode.cpp QL/ql_joinNode.cpp QL/ql_projNode.cpp QL/ql_selNode.cpp
        Parse/lexAnalyser.cpp Parse/perfectHash.cpp Parse/lex_test.cpp Parse/SyntaxAnalyser.cpp Parse/SyntaxAnalyser.h Parse/parser.cpp)

set(DBCREATE_SOURCE
        BM/bm_hashTable.cpp BM/bm_hashTable.h BM/bm_buffermgr.cpp BM/bm_buffermgr.h PF/pf_filemgr.cpp PF/pf_exception.h RM/rm.cpp RM/rm.h RM/rm_fileHandler.cpp RM/rm_pageHandler.cpp
        RM/rm_fileScan.cpp IX/ix.h IX/ix_manager.cpp IX/ix_IndexHandler.cpp IX/IX_pageHandler.cpp IX/ix_Key.cpp IX/ix_IndexScan.cpp SM/sm_manager.cpp SM/printer.h SM/printer.cpp
        dbcreate.cpp QL/ql.cpp QL/ql.h QL/ql_node.cpp QL/ql_relNode.cpp QL/ql_joinNode.cpp QL/ql_projNode.cpp QL/ql_selNode.cpp)

set(DBDESTROY_SOURCE
        BM/bm_hashTable.cpp BM/bm_hashTable.h BM/bm_buffermgr.cpp BM/bm_buffermgr.h PF/pf_filemgr.cpp PF/pf_exception.h RM/rm.cpp RM/rm.h RM/rm_fileHandler.cpp RM/rm_pageHandler.cpp
        RM/rm_fileScan.cpp IX/ix.h IX/ix_manager.cpp IX/ix_IndexHandler.cpp IX/IX_pageHandler.cpp IX/ix_Key.cpp IX/ix_IndexScan.cpp SM/sm_manager.cpp SM/printer.h SM/printer.cpp QL/ql.h QL/ql_node.cpp QL/ql_relNode.cpp QL/ql_joinNode.cpp QL/ql_projNode.cpp QL/ql_selNode.cpp QL/ql.cpp
        dbdestroy.cpp)
add_executable(miniSqlFrom0 ${SOURCE_FILES})
add_executable(dbcreate ${DBCREATE_SOURCE})
add_executable(dbdestroy ${DBDESTROY_SOURCE})