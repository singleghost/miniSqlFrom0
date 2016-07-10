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
    PageDescTable[firstUsed].prev = slot;
    PageDescTable[slot].prev = INVALID_SLOT;
    PageDescTable[slot].next = firstUsed;
    firstUsed = slot;
    return 0;
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
        PageDescTable[temp].pinCount = 0;
        PageDescTable[temp].isdirty = false;
        UsedLinkHead(temp);
        firstUsed = temp;
        if(lastUsed == -1) lastUsed = temp;
        ppointer = PageDescTable[temp].page_addr;
        return 0;
    } else {
        //替换掉最早被使用过的页
        int _slot;
        for(_slot = lastUsed; _slot != INVALID_SLOT; _slot = PageDescTable[_slot].prev) {
            if(PageDescTable[_slot].pinCount == 0) {
                if(PageDescTable[_slot].isdirty) {
                    int offset = PF_FILE_Hdr + pageNum * pageSize;
                    lseek(PageDescTable[_slot].fd, offset, L_SET);
                    write(PageDescTable[_slot].fd, PageDescTable[_slot].page_addr, pageSize);
                    PageDescTable[_slot].isdirty = false;
                }
                hashTable->Delete(PageDescTable[_slot].fd, PageDescTable[_slot].pageNum);

                if(_slot == lastUsed) lastUsed = PageDescTable[_slot].prev;
                UnlinkFromFree(_slot);
                hashTable->Insert(fd, pageNum, _slot);
                UsedLinkHead(_slot);
                PageDescTable[_slot].fd = fd;
                PageDescTable[_slot].pageNum = pageNum;
                PageDescTable[_slot].isdirty = false;
                PageDescTable[_slot].pinCount = 0;
                ppointer = PageDescTable[_slot].page_addr;
                return 0;
            }
        }

        return PF_NO_FREE_BUF_WARNING;

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


char* BM_BufferMgr::GetPage(int fd, int pageNum) {
    int slot;
    hashTable->Find(fd, pageNum, slot);
    if(slot != INVALID_SLOT) {
        PageDescTable[slot].pinCount += 1;
        return PageDescTable[slot].page_addr;
    }
    return NULL;
}


RC BM_BufferMgr::UnPinPage(int fd, int pageNum) {
    int slot;
    hashTable->Find(fd, pageNum, slot);
    if(slot != INVALID_SLOT) {
        PageDescTable[slot].pinCount -= 1;
        return 0;
    }
    return -1;
}

RC BM_BufferMgr::FreePage(int fd, int pageNum) {
    int slot;
    hashTable->Find(fd, pageNum, slot);

    UnlinkFromUsed(slot);
    FreeLinkHead(slot);
    hashTable->Delete(fd, pageNum);
    return 0;
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
