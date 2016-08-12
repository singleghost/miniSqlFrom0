//
// Created by 谢俊东 on 16/7/13.
//

#include <cassert>
#include <zconf.h>
#include <fcntl.h>
#include "rm.h"

RC RM_Manager::CreateFile(string filename, int recordSize){
    assert(recordSize >= sizeof(int) && recordSize <= MAX_RECORDSIZE);  //recordSize至少要有4字节,用来存放record free list的下标
    pfm.CreateFile(filename);
    int fd = open(filename.c_str(), O_RDWR, 0666);
    if(fd < 0) return FILE_OPEN_ERROR;
    lseek(fd, sizeof(fileHeader), L_SET);
    RM_FileHeader rm_fileHeader;
    rm_fileHeader.RecordSize = recordSize;
    rm_fileHeader.nMaxRecordsPerPage = (PAGE_SIZE - sizeof(pageHeader) - sizeof(RM_PageHeader)) / (recordSize + 1);
    write(fd, &rm_fileHeader, sizeof(RM_FileHeader));
    close(fd);
    return 0;
}

RC RM_Manager::OpenFile(string filename, RM_FileHandler &rm_fileHandler) {
    FileHandler pf_fileHandler;
    if( pfm.OpenFile(filename, pf_fileHandler) == FILE_OPEN_ERROR)
        return FILE_OPEN_ERROR;
    rm_fileHandler.bHeaderModified = false;
    rm_fileHandler.pf_fileHandler = pf_fileHandler;
    rm_fileHandler.fd = pf_fileHandler.getFd();

    lseek(rm_fileHandler.fd, sizeof(fileHeader), L_SET);
    read(rm_fileHandler.fd, &rm_fileHandler.rm_fh, sizeof(RM_FileHeader));
    return 0;
}

RC RM_Manager::CloseFile(RM_FileHandler &rm_fileHandler) {
    if(rm_fileHandler.bHeaderModified) {
        lseek(rm_fileHandler.fd, sizeof(fileHeader), L_SET);
        write(rm_fileHandler.fd, &rm_fileHandler.rm_fh, sizeof(RM_FileHeader));
    }
    pfm.CloseFile(rm_fileHandler.pf_fileHandler);
    return 0;
}

RC RM_Manager::DestroyFile(string filename) {
    return pfm.DestroyFile(filename);

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

const char *rm_error_msg[] = { "RM: record not in use", "RM: EOF", "RM: page record eof"};
void RM_PrintError(RC rc) {
    printf("Error: %s\n", rm_error_msg[START_RM_ERR - rc]);
}
