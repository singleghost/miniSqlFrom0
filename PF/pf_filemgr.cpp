//
// Created by root on 16-6-29.
//


#include <fcntl.h>
#include <zconf.h>
#include <cstring>
#include <cassert>
#include "pf_filemgr.h"
#include "pf_exception.h"

//PF_Manager

RC PF_Manager::CreateFile(string filename) {
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
    if(fd < 0)
        return FILE_CREATE_ERROR;
    char fileHeaderPage[PAGE_SIZE];
    fileHeader *fh = (fileHeader *) fileHeaderPage;
    fh->firstFreePage = PAGELISTEND;
    fh->numOfPages = 0;

    lseek(fd, 0, L_SET);
    write(fd, fileHeaderPage, PAGE_SIZE);
    close(fd);
    return 0;
}


RC PF_Manager::OpenFile(string filename, FileHandler &fhdl){
    fhdl.fd = open(filename.c_str(), O_RDWR, 0666);
    if(fhdl.fd < 0) return FILE_OPEN_ERROR;
        lseek(fhdl.fd, 0, L_SET);
    read(fhdl.fd, (char *) &fhdl.fh, sizeof(fileHeader));
    fhdl.bufferMgr = this->bufferMgr;
    return 0;
}

RC PF_Manager::CloseFile(FileHandler &fhandler){
    if(fhandler.bFileHeaderChanged) {
        //如果文件头被改变的话, 将被改变的文件头写回文件
        lseek(fhandler.fd, 0, L_SET);
        write(fhandler.fd, &fhandler.fh, sizeof(fileHeader));
    }
    bufferMgr->FlushPages(fhandler.fd);
//    fhandler.ForcePages();
    close(fhandler.fd); //关闭文件描述符

    fhandler.fd = -1;   //把fd改成一个不合法的值,防止被误用
    return 0;
}

RC PF_Manager::DestroyFile(string filename) {
    unlink(filename.c_str());
    return 0;
}

//FileHandler
FileHandler::FileHandler(const FileHandler &fileHandle) {
    this->bFileHeaderChanged = fileHandle.bFileHeaderChanged;
    this->bufferMgr = fileHandle.bufferMgr;
    this->fd = fileHandle.fd;
    this->fh = fileHandle.fh;
}

FileHandler& FileHandler::operator=(const FileHandler &fileHandle) {
    if(this == &fileHandle) {
        return *this;
    } else {
        this->fd = fileHandle.fd;
        this->fh = fileHandle.fh;
        this->bufferMgr = fileHandle.bufferMgr;
        this->bFileHeaderChanged = fileHandle.bFileHeaderChanged;
        return *this;
    }
}

RC FileHandler::GetFirstPage(PageHandler &pageHandle) const {
    return GetNextPage(-1, pageHandle);
}

//先在内存中寻找，如果找不到就读入内存中
RC FileHandler::GetThisPage(int pageNum, PageHandler &phandler) const throw(no_free_buffer_exp){
    if(fh.numOfPages == 0) return PF_NO_PAGE_IN_FILE;
    assert(pageNum >= 0 && pageNum < fh.numOfPages);
    char *page_addr;
    if(bufferMgr->GetPage(fd, pageNum, page_addr) == BM_NO_FREE_BUF_WARNING) throw no_free_buffer_exp();
    if (((pageHeader *)page_addr)->nextFree != PAGEINUSE ) return PAGE_NOT_IN_USE;
    memcpy(&phandler.phdr, page_addr, sizeof(pageHeader));
    phandler.data = page_addr + sizeof(pageHeader);

    return 0;
}

RC FileHandler::GetNextPage(int pageNum, PageHandler &pageHandle) const {
    int current;
    for (current = pageNum + 1; current < fh.numOfPages; current++) {
        if(GetThisPage(current, pageHandle) == PAGE_NOT_IN_USE) continue;
        else return 0;
    }
    throw page_not_found_exception();
}

RC FileHandler::GetLastPage(PageHandler &pageHandle) const {
    for(int current = fh.numOfPages - 1; current >= 0; current--) {
        if(GetThisPage(current, pageHandle) == PAGE_NOT_IN_USE) continue;
        else return 0;
    }
    throw page_not_found_exception();
}

RC FileHandler::GetPrevPage(int current, PageHandler &pageHandle) const {
    for(int pageIndex = current - 1; pageIndex >= 0; pageIndex--) {
        if(GetThisPage(pageIndex, pageHandle) == PAGE_NOT_IN_USE) continue;
        else return 0;
    }
    throw page_not_found_exception();
}

RC FileHandler::AllocatePage(PageHandler &pageHandle) {
    char *page_addr;
    int pageIdx;
    if(fh.firstFreePage != PAGELISTEND) {
        //优先从free list中找到空闲页
        pageIdx = fh.firstFreePage;
        if( bufferMgr->GetPage(fd, pageIdx, page_addr) == BM_NO_FREE_BUF_WARNING) return BM_NO_FREE_BUF_WARNING;
        fh.firstFreePage = ((pageHeader *)page_addr)->nextFree;
        ((pageHeader *)page_addr)->nextFree = PAGEINUSE;
        bFileHeaderChanged = true;
    } else {
        //如果没有则新分配一个Page
        pageIdx = fh.numOfPages;
        if( bufferMgr->AllocatePage(fd, pageIdx, page_addr) == BM_NO_FREE_BUF_WARNING) {
            printf("no free buffer\n");
            exit(233);
//            return BM_NO_FREE_BUF_WARNING;
        }
        fh.numOfPages++;
        bFileHeaderChanged = true;
    }
    if(page_addr != NULL) {
        pageHandle.phdr.pageNum = pageIdx;
        pageHandle.phdr.nextFree = PAGEINUSE;
        memcpy(page_addr, &pageHandle.phdr, sizeof(pageHeader));
        pageHandle.data = page_addr + sizeof(pageHeader);
        MarkDirty(pageIdx);     //mark dirty 因为我们改变了文件头
        return 0;
    }
    return -1;
}


RC FileHandler::DisposePage(int pageNum) {
    char *page_addr;
    bufferMgr->GetPage(fd, pageNum, page_addr);
    ((pageHeader *)page_addr)->nextFree = fh.firstFreePage;
    fh.firstFreePage = pageNum;
    bFileHeaderChanged = true;
    UnpinPage(pageNum);
    MarkDirty(pageNum); //mark dirty因为我们改变了文件头

    return 0;
}

RC FileHandler::MarkDirty(int pageNum) const {
    bufferMgr->MarkDirty(fd, pageNum);
    return 0;
}

RC FileHandler::ForcePages(int pageNum) const {
    if(pageNum == ALL_PAGES) {
        for(int i = 0; i < fh.numOfPages; i++) {
            bufferMgr->ForcePage(fd, i);
        }
        return 0;
    } else {
        bufferMgr->ForcePage(fd, pageNum);
        return 0;
    }
}

RC FileHandler::UnpinPage(int pageNum) const {
    bufferMgr->UnPinPage(fd, pageNum);
    return 0;
}

//PageHandler

PageHandler& PageHandler::operator=(const PageHandler &phandler) {
    if(this == &phandler) {
        return *this;
    } else {
        this->phdr = phandler.phdr;
        this->data = phandler.data;
        return *this;
    }
}

