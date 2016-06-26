//
// Created by root on 16-6-5.
//

#include <cstring>
#include <fcntl.h>
#include <zconf.h>
#include <cstdio>
#include "bm_buffermgr.h"
#include "bm_error.h"



BM_BufferMgr::BM_BufferMgr(int numOfPages) {
    PageDescTable = new bufferPageDesc[numOfPages];
    for (auto i = 0; i < numOfPages; i++) {
        PageDescTable[i].page_addr = (char *) malloc(PAGE_SIZE);
        PageDescTable[i].next = i + 1;
        PageDescTable[i].prev = i - 1;
    }
    PageDescTable[numOfPages - 1].next = -1;
    PageDescTable[0].prev = -1;
    firstFree = 0;
    firstUsed = -1;
    lastUsed = -1;

    hashTable = new BM_HashTable(40);
}

RC BM_BufferMgr::AllocatePage(int fd, int pageNum, char *&ppointer) {

    int RC = 0;
    int slot;

    //先在hash表中寻找是否已经存在，存在的话直接返回
    hashTable->Find(fd, pageNum, slot);
    if (slot != INVALID_SLOT) {
        ppointer = PageDescTable[slot].page_addr;
        return RC;
    }

    //free list中寻找空闲的页，如没有则替换掉最近被使用的页
    if (firstFree != NO_MORE_FREE_PAGE) {
        int temp = firstFree;
        firstFree = PageDescTable[firstFree].next;
        hashTable->Insert(fd, pageNum, temp);
        PageDescTable[temp].fd = fd;
        PageDescTable[temp].pageNum = pageNum;
        ppointer = PageDescTable[temp].page_addr;
        return 0;
    } else {
        //替换掉最早被使用过的页
        int _slot;
        for(_slot = lastUsed; _slot != INVALID_SLOT; _slot = PageDescTable[lastUsed].prev) {
            if(PageDescTable[_slot].pinCount == 0) {
                if(PageDescTable[_slot].isdirty) {
                    int offset = PF_FILE_Hdr + pageNum * pageSize;
                    lseek(fd, offset, L_SET);
                    write(fd, PageDescTable[_slot].page_addr, pageSize);
                    PageDescTable[_slot].isdirty = false;
                }

                Unlink(_slot);
                hashTable->Insert(fd, pageNum, _slot);
                LinkHead(_slot);
            }
        }

        if(_slot == INVALID_SLOT) {
            return PF_NO_FREE_BUF_WARNING;
        }


    }
}

BM_BufferMgr::~BM_BufferMgr() {
    if (PageDescTable) {
        for (auto i = 0; i < numOfPages; i++) {
            if (PageDescTable[i].page_addr)
                free(PageDescTable[i].page_addr);
        }
    }
    delete[] PageDescTable;
    delete hashTable;
}

RC BM_BufferMgr::AllocatePage(int fd, int pageNum, char *&ppointer) {
    return 0;
}





