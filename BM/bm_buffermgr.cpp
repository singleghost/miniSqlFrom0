//
// Created by root on 16-6-5.
//

#include <cstring>
#include <fcntl.h>
#include <zconf.h>
#include <cstdio>
#include <stdexcept>
#include "bm_buffermgr.h"
#include "bm.h"

using namespace std;

BM_BufferMgr::BM_BufferMgr(int numOfPages) {
    if(numOfPages <= 0 || numOfPages >= MAX_BUFFER_POOL_SIZE) throw invalid_argument("argument numOfPages not valid");
    this->numOfPages = numOfPages;
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

RC BM_BufferMgr::UnlinkFromFree(int slot){
    int prevSlot = PageDescTable[slot].prev;
    if(prevSlot != -1)
        PageDescTable[prevSlot].next = PageDescTable[slot].next;
    int nextSlot = PageDescTable[slot].next;
    if(nextSlot != -1)
        PageDescTable[nextSlot].prev = PageDescTable[slot].prev;

    return 0;
}

RC BM_BufferMgr::UnlinkFromUsed(int slot) {
    int prevSlot = PageDescTable[slot].prev;
    if(prevSlot != -1)
        PageDescTable[prevSlot].next = PageDescTable[slot].next;
    int nextSlot = PageDescTable[slot].next;
    if(nextSlot != -1)
        PageDescTable[nextSlot].prev = PageDescTable[slot].prev;
    else lastUsed = prevSlot;

    return 0;
}

RC BM_BufferMgr::UsedLinkHead(int slot) {
    if(lastUsed == -1) lastUsed = slot;
    PageDescTable[firstUsed].prev = slot;
    PageDescTable[slot].prev = INVALID_SLOT;
    PageDescTable[slot].next = firstUsed;
    firstUsed = slot;
    return 0;
}

RC BM_BufferMgr::AllocatePage(int fd, int pageNum, char *&ppointer) {

    int slot;
    //先在hash表中寻找是否已经存在，存在的话返回错误

    hashTable->Find(fd, pageNum, slot);
    if(slot != INVALID_SLOT) {
        //如果Page已经被读入内存

        throw page_exist_exception();
    } else {
        //如果内存中没有该Page
        InternalAlloc(slot);
        InitPageDesc(fd, pageNum, slot);
        hashTable->Insert(fd, pageNum, slot);
        ppointer = PageDescTable[slot].page_addr;
        PageDescTable[slot].pinCount++;
    }
    return 0;
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


RC BM_BufferMgr::GetPage(int fd, int pageNum, char *&ppointer){
    int slot;
    hashTable->Find(fd, pageNum, slot);
    if(slot != INVALID_SLOT) {
        //如果Page已经被读入内存
        PageDescTable[slot].pinCount += 1;
        ppointer = PageDescTable[slot].page_addr;
    } else {
        //如果内存中没有该Page
        if( InternalAlloc(slot) == BM_NO_FREE_BUF_WARNING ) return BM_NO_FREE_BUF_WARNING;
        InitPageDesc(fd, pageNum, slot);
        ReadPage(fd, pageNum, PageDescTable[slot].page_addr);
        hashTable->Insert(fd, pageNum, slot);
        PageDescTable[slot].pinCount++;
        ppointer = PageDescTable[slot].page_addr;
    }
    return 0;
}

RC BM_BufferMgr::InternalAlloc(int &slot) {
    if(firstFree != INVALID_SLOT) {
        slot = firstFree;
        firstFree = PageDescTable[firstFree].next;
        UsedLinkHead(slot);
        return 0;
    }
    int _slot;
    for(_slot = lastUsed; _slot != INVALID_SLOT; _slot = PageDescTable[_slot].prev) {
        if(PageDescTable[_slot].pinCount == 0) {
            if(PageDescTable[_slot].isdirty) {
                int offset = PF_FILE_Hdr + PageDescTable[_slot].pageNum * pageSize;
                lseek(PageDescTable[_slot].fd, offset, L_SET);
                write(PageDescTable[_slot].fd, PageDescTable[_slot].page_addr, pageSize);
                PageDescTable[_slot].isdirty = false;
            }
            hashTable->Delete(PageDescTable[_slot].fd, PageDescTable[_slot].pageNum);

            UnlinkFromUsed(_slot);
            UsedLinkHead(_slot);
            slot = _slot;
            return 0;
        }
    }

    return BM_NO_FREE_BUF_WARNING;
}

RC BM_BufferMgr::InitPageDesc(int fd, int pageNum, int slot) {

    PageDescTable[slot].fd = fd;
    PageDescTable[slot].pageNum = pageNum;
    PageDescTable[slot].isdirty = false;
    PageDescTable[slot].pinCount = 0;
    return 0;
}

RC BM_BufferMgr::UnPinPage(int fd, int pageNum) {
    int slot;
    hashTable->Find(fd, pageNum, slot);
    if(slot != INVALID_SLOT) {
        if(PageDescTable[slot].pinCount > 0)
            PageDescTable[slot].pinCount -= 1;
        return 0;
    }
    return -1;
}

RC BM_BufferMgr::FreeLinkHead(int slot) {
    if(firstFree != -1) {
        PageDescTable[firstFree].prev = slot;
    }
    PageDescTable[slot].next = firstFree;
    PageDescTable[slot].prev = -1;
    firstFree = slot;
    return 0;

}

void BM_BufferMgr::ForcePage(int fd, int pageNum){
    int _slot;
    hashTable->Find(fd, pageNum, _slot);
    if(PageDescTable[_slot].isdirty) {
        int offset = PF_FILE_Hdr + pageNum * pageSize;
        lseek(PageDescTable[_slot].fd, offset, L_SET);
        write(PageDescTable[_slot].fd, PageDescTable[_slot].page_addr, pageSize);
        PageDescTable[_slot].isdirty = false;
    }
}

void BM_BufferMgr::writeBackAllDirty(int fd) {
    int temp = firstUsed;
    while(temp != INVALID_SLOT) {
        if(PageDescTable[temp].fd == fd) {
            ForcePage(fd, PageDescTable[temp].pageNum);
        }
        temp = PageDescTable[temp].next;
    }
}

void BM_BufferMgr::MarkDirty(int fd, int pageNum) {
    int slot;
    hashTable->Find(fd, pageNum, slot);
    PageDescTable[slot].isdirty = true;
}

void BM_BufferMgr::ReadPage(int fd, int pageNum, char *dest) {
    lseek(fd, PF_FILE_Hdr + pageNum * PAGE_SIZE, SEEK_SET);
    read(fd, dest, PAGE_SIZE);
}

void BM_BufferMgr::WritePage(int fd, int pageNum, char *src) {
    lseek(fd, PF_FILE_Hdr + pageNum * PAGE_SIZE, SEEK_SET);
    write(fd, src, PAGE_SIZE);
}

void BM_BufferMgr::PrintPageDescTable() {
    printf("next\tprev\tpinCount\tisDirty\tPageNum\tfd\tpageContent\n");
    for(int i = 0; i < numOfPages; i++) {
        printf("%4d\t%4d\t%8d\t%7d\t%7d\t%2d\t%11s", PageDescTable[i].next,
               PageDescTable[i].prev, PageDescTable[i].pinCount, PageDescTable[i].isdirty, PageDescTable[i].pageNum,
               PageDescTable[i].fd, PageDescTable[i].page_addr);
    }
}
