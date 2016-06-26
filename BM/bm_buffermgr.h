//
// Created by root on 16-6-5.
//

#ifndef MINISQLFROM0_BM_BUFFERMGR_H
#define MINISQLFROM0_BM_BUFFERMGR_H

#include "bm_hashTable.h"
#include "../minisql.h"

struct bufferPageDesc{
    char *page_addr;
    int next;
    int prev;
    int pinCount;
    bool isdirty;
    int pageNum;
    int fd;
};

class BM_BufferMgr {
private:
    bufferPageDesc *PageDescTable;
    int firstFree;
    int firstUsed; //MRU Most Recent Used Page
    int lastUsed; //LRU Least Recent Userd Page
    BM_HashTable *hashTable;

    int numOfPages;
    const int pageSize = 4096;

public:
    BM_BufferMgr(int numOfPages);

    ~BM_BufferMgr();

    RC AllocatePage(int fd, int pageNum, char *&ppointer);
    RC FreePage(int fd, int pageNum);
    RC PinPage(int fd, int pageNum);
    RC UnPinPage(int fd, int pageNum);


private:
    RC LinkHead(int slot);
    RC UnLink(int slot);
};


#endif //MINISQLFROM0_BM_BUFFERMGR_H
