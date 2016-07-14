//
// Created by 谢俊东 on 16/7/13.
//

#include <cassert>
#include <zconf.h>
#include "rm.h"

void RM_Manager::CreateFile(string filename, int recordSize){
    assert(recordSize >= sizeof(int) && recordSize <= MAX_RECORDSIZE);
    FileHandler fHandler;
    PageHandler pageHandler;
    pfm.CreateFile(filename);
    FILE *fp = fopen(filename.c_str(), "a");
    fseek(fp, sizeof(fileHeader), SEEK_SET);
    RM_FileHeader rm_fileHeader;
    rm_fileHeader.RecordSize = recordSize;
    rm_fileHeader.numOfPages = 0;
    rm_fileHeader.nMaxRecordsPerPage = (PAGE_SIZE - sizeof(pageHeader) - sizeof(RM_PageHeader)) / (recordSize + 1);
    fwrite(&rm_fileHeader, 1, sizeof(RM_FileHeader), fp);
    fclose(fp);
}

void RM_Manager::OpenFile(string filename, RM_FileHandler &fileHandler) {
    FileHandler pf_fileHandler;
    pfm.OpenFile(filename, pf_fileHandler);
    fileHandler.bHeaderModified = false;
    fileHandler.pf_fileHandler = pf_fileHandler;
    fileHandler.fd = pf_fileHandler.getFd();
    lseek(fileHandler.fd, sizeof(fileHeader), L_SET);
    read(fileHandler.fd, &fileHandler.rm_fh, sizeof(RM_FileHeader));
}

void RM_Manager::CloseFile(RM_FileHandler &rm_fileHandler) {
    if(rm_fileHandler.bHeaderModified) {
        lseek(rm_fileHandler.fd, sizeof(fileHeader), L_SET);
        write(rm_fileHandler.fd, &rm_fileHandler.rm_fh, sizeof(RM_FileHeader));
    }
    pfm.CloseFile(rm_fileHandler.pf_fileHandler);
}

void RM_Manager::DestroyFile(string filename) {
    pfm.DestroyFile(filename);
}


//RM_Record
RM_Record::RM_Record() {
    this->content = NULL;
    this->recordSize = -1;
}

//复制构造函数
RM_Record::RM_Record(const RM_Record &rm_rec) {
    this->recordSize = rm_rec.recordSize;
    this->rid = rm_rec.rid;
    this->content = new char[this->recordSize];
    memcpy(this->content, rm_rec.content, recordSize);
}

//赋值函数
RM_Record& RM_Record::operator=(const RM_Record &rec) {
    if(this == &rec) {
        return *this;
    } else {
        if(this->content)
            delete [] this->content;
        this->recordSize = rec.recordSize;
        this->rid = rec.rid;
        this->content = new char[this->recordSize];
        memcpy(this->content, rec.content, this->recordSize);
        return *this;
    }
}