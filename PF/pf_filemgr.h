//
// Created by root on 16-6-29.
//

#ifndef MINISQLFROM0_PF_FILEMGR_H
#define MINISQLFROM0_PF_FILEMGR_H

#include <iostream>
#include "../minisql.h"
#include "../BM/bm_buffermgr.h"
#include "pf_exception.h"
using std::string;

#define PAGE_NOT_IN_USE -1      //返回码
#define BUFFER_POOL_SIZE 100    //缓冲区的大小,同时最多能放多少页
#define PAGELISTEND -1          //用于pageHeader中nextFree字段,表示free list的末尾
#define PAGEINUSE   -2          //用于pageHeader中的nextFree字段,表示当前页被使用

#define ALL_PAGES -1            //用于默认参数

struct pageHeader {
    int nextFree;   //下一个free page
    int pageNum;        //页号
};  //页头

struct fileHeader {
    int firstFreePage; //page free list的表头,第一个空闲页
    int numOfPages;         //文件总共有多少页，包括使用中和空闲的
};  //文件头,为了方便,占据PAGE SIZE的空间



class PageHandler
{
    friend class FileHandler;
private:
    pageHeader phdr;    //页头
    char *data;     //指向页中实际存储的数据
public:
    PageHandler() { data = NULL; }
    PageHandler(const PageHandler &phandler) { this->phdr = phandler.phdr; data = phandler.data; }
    PageHandler & operator= (const PageHandler &phandler);
    char * GetDataPtr() {
        //得到指向页中数据的指针,页中数据不包括页头
        return data;
    }

    int GetPageNum() {
        //得到页的编号
        return phdr.pageNum;
    }

};



class FileHandler
{
    friend class PF_Manager;
private:
    int fd;         //文件描述符
    fileHeader fh;  //文件头
    BM_BufferMgr *bufferMgr;
    bool bFileHeaderChanged;    //标志文件头是否被改变

public:
    FileHandler () :fd(-1), bufferMgr(NULL), bFileHeaderChanged(false) {}
    FileHandler  (const FileHandler &fileHandle);   // Copy constructor
    FileHandler& operator=(const FileHandler &fileHandle); // Overload =

    RC GetFirstPage   (PageHandler &pageHandle) const;   // Get the first page
    RC GetLastPage    (PageHandler &pageHandle) const;   // Get the last page
    RC GetNextPage    (int pageNum, PageHandler &pageHandle) const; // Get the next page
    RC GetPrevPage    (int current, PageHandler &pageHandle) const; // Get the previous page
    RC GetThisPage    (int pageNum, PageHandler &pageHandle) const throw(no_free_buffer_exp); // Get a specific page

    RC AllocatePage   (PageHandler &pageHandle);         // Allocate a new page
    RC DisposePage    (int pageNum);                   // 将某个Page加入page free list
    RC MarkDirty      (int pageNum) const;             // page上无论什么地方有改动都要mark dirty
    RC UnpinPage      (int pageNum) const;             // 将page的pinCount减一
    RC ForcePages     (int pageNum = ALL_PAGES) const; // Write dirty page(s) to disk, 默认参数写回所有dirty page

    //getter and setter
    int getFd() const { return fd; }

    //测试用函数
    int GetNumOfPages() const { return fh.numOfPages; } //返回文件有多少页,包括空闲和使用中的
};

class PF_Manager
{
private:
    BM_BufferMgr *bufferMgr;
public:
    PF_Manager() { bufferMgr = new BM_BufferMgr(BUFFER_POOL_SIZE); }
    ~PF_Manager() { delete bufferMgr; }

    RC CreateFile(string filename);         //在磁盘上创建文件,初始化文件头,文件头占据一个Page的空间
    RC OpenFile(string filename, FileHandler &fh);  //打开文件, fileHandler指向被打开的文件
    RC CloseFile(FileHandler &fh);          //关闭文件
    RC DestroyFile(string filename);        //删除磁盘上的文件

    //调试用
    BM_BufferMgr *GetBufferManager() { return bufferMgr; }
};

#endif //MINISQLFROM0_PF_FILEMGR_H
