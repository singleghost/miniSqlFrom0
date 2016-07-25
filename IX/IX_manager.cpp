//
// Created by 谢俊东 on 16/7/17.
//

#include <sstream>
#include <fcntl.h>
#include <sys/file.h>
#include <zconf.h>
#include "ix.h"

RC IX_Manager::CreateIndex(string filename, int indexNo, AttrType attrType, int attrLength) {
    stringstream ss;
    ss << indexNo;
    string ext;
    ss >> ext;
    string real_name = filename + "." + ext;
    pfm.CreateFile(real_name);

    int fd = open(real_name.c_str(), O_RDWR, 0666);
    if(fd < 0) return FILE_OPEN_ERROR;
    lseek(fd, sizeof(fileHeader), L_SET);
    IX_fileHeader ix_fileHeader;
    ix_fileHeader.attrType = attrType;
    ix_fileHeader.attrLength = attrLength;
    ix_fileHeader.IndexNo = indexNo;
    ix_fileHeader.rootNode = -1;
    ix_fileHeader.nMaxPtrLeafPage = (PAGE_SIZE - sizeof(pageHeader) - sizeof(IX_pageHeader))/ (attrLength + sizeof(RID)) - 1;//故意减一,留出一个空位,简化算法
    ix_fileHeader.nMaxPtrInteriorPage = (PAGE_SIZE - sizeof(pageHeader) - sizeof(IX_pageHeader)) / (sizeof(PagePtr) + attrLength) - 1;//故意减一,留出一个空位,简化算法
    //调试用
//    ix_fileHeader.nMaxPtrInteriorPage = 5;
//    ix_fileHeader.nMaxPtrLeafPage = 50;
    write(fd, &ix_fileHeader, sizeof(IX_fileHeader));
    close(fd);
    return 0;
}

RC IX_Manager::OpenIndex(string filename, int indexNo, IX_IndexHandler &ix_handler) {
    stringstream ss;
    ss << indexNo;
    string ext;
    ss >> ext;
    string real_name = filename + "." + ext;
    FileHandler fileHandler;
    pfm.OpenFile(real_name, fileHandler);
    ix_handler.pf_fileHandler = fileHandler;
    ix_handler.fd = fileHandler.getFd();
    lseek(ix_handler.fd, sizeof(fileHeader), L_SET);
    read(ix_handler.fd, &ix_handler.ix_fh, sizeof(IX_fileHeader));

    return 0;
}

RC IX_Manager::CloseIndex(IX_IndexHandler &ix_handler) {

    if(ix_handler.bHeaderModified) {
        lseek(ix_handler.fd, sizeof(fileHeader), L_SET);
        write(ix_handler.fd, &ix_handler.ix_fh, sizeof(IX_fileHeader));
    }
    pfm.CloseFile(ix_handler.pf_fileHandler);
    return 0;
}

RC IX_Manager::DestroyIndex(string filename, int indexNo) {
    stringstream ss;
    ss << indexNo;
    string ext;
    ss >> ext;
    //将indexNo转换为string作为文件的扩展名
    return pfm.DestroyFile(filename + "." + ext);
}
