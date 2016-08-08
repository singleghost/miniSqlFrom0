//
// Created by root on 16-6-5.
//

#ifndef MINISQLFROM0_BM_HASHTABLE_H
#define MINISQLFROM0_BM_HASHTABLE_H

#include <cstdlib>
#include "../minisql.h"
#include "bm.h"

struct TableEntry {
    TableEntry *next;   //指向下一条记录的指针
    int fd;
    int pageNum;
    int slot;           //slot 为Page Description Table的下标
};

/* 通过链表的方式来解决冲突 */
class BM_HashTable {

private:
    TableEntry **HashTable;
    int numOfBuckets;   //hashTable的大小

public:
    BM_HashTable(int);

    ~BM_HashTable();

    RC Insert(int fd, int pageNum, int slot);   //插入

    RC Find(int fd, int pageNum ,int &slot);    //查找,结果存入slot

    RC Delete(int fd, int pageNum);             //删除

    int Hash(int fd, int pageNum);  //简易的ｈａｓｈ函数实现
};

#endif //MINISQLFROM0_BM_HASHTABLE_H
