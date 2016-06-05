//
// Created by root on 16-6-5.
//

#ifndef MINISQLFROM0_BM_HASHTABLE_H
#define MINISQLFROM0_BM_HASHTABLE_H

#include <cstdlib>
#include "../minisql.h"
#include "bm_error.h"

struct TableEntry {
    TableEntry *next;   //指向前一条记录的指针
    int fd;
    int pageNum;
    int slot;
};

/* 通过链表的方式来解决冲突 */
class BM_HashTable {

private:
    const int INVALID_SLOT = -1;
    TableEntry **HashTable;
    int numOfBuckets;

public:
    BM_HashTable(int);

    ~BM_HashTable();

    RC Insert(int fd, int pageNum, int slot);

    RC Find(int fd, int pageNum ,int &slot);

    RC Delete(int fd, int pageNum);

    int Hash(int fd, int pageNum);  //简易的ｈａｓｈ函数实现
};

#endif //MINISQLFROM0_BM_HASHTABLE_H
