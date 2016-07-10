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
    RC UnPinPage(int fd, int pageNum);
    void ForcePage(int fd, int pageNum);    //把一页写回硬盘,但不从buffer pool里移除
    void writeBackAllDirty(int fd);         //把某文件的所有dirty page写回磁盘, 但不从buffer pool里移除
    char *GetPage(int fd, int pageNum);
    void MarkDirty(int fd, int pageNum);

    //may be used for test only!
    void ReadPage(int fd, int pageNum, char *dest);
    void WritePage(int fd, int pageNum , char *src);
    void PrintPageDescTable();
private:
    RC UsedLinkHead(int slot);  //将slot放在usedlist的头
    RC UnlinkFromFree(int slot);    //将slot从freelist中移除
    RC UnlinkFromUsed(int slot);    //将slot从usedlist中移除
    RC FreeLinkHead(int slot);  //将slot放在freelist的表头
};


#endif //MINISQLFROM0_BM_BUFFERMGR_H
