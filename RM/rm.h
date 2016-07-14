//
// Created by 谢俊东 on 16/7/13.
//

#ifndef MINISQLFROM0_RM_MANAGER_H
#define MINISQLFROM0_RM_MANAGER_H

#include "../minisql.h"
#include "../PF/pf_filemgr.h"

#define RECORD_FREE_LIST_END -1
#define SLOT_IN_USE 1
#define SLOT_FREE 0

//返回码
#define RECORD_NOT_IN_USE -1

struct RM_PageHeader {
    int firstFreeRec;   //第一个空闲的record的位置
    int NumOfRecords;   //当前页共有多少Record, 不包括空闲的
};  //RM模块的页头

struct RM_FileHeader {
    int RecordSize;         //每条记录的长度
    int nMaxRecordsPerPage; //一页最多能存储多少条记录
    int numOfPages;         //文件头不算,总共有多少页
};  //RM模块的文件头

//用slot和pageNum来标识record,RID是对slot和pageNum的简单封装
class RID {
    private:
        int slot;
        int pageNum;
    public:
        RID() :pageNum(-1), slot(-1) {}
        RID(int pageNum, int slot) : pageNum(pageNum), slot(slot) {}
    //...采用编译器默认的复制构造函数和赋值函数
        ~RID() {}
    //getter and setter
        int getPageNum() const { return pageNum; }
        void setPageNum(int pageNum) { this->pageNum = pageNum; }
        int getSlot() const { return slot; }
        void setSlot(int slot) { this->slot = slot; }
};

//RM_Record objects should contain copies of records from the buffer pool, not records in the buffer pool itself.
class RM_Record {
private:
    RID rid;
    char *content;
    int recordSize;
public:
    RM_Record();
    RM_Record(char *data, RID &rid, int recordSize) : rid(rid), recordSize(recordSize) {
        content = new char[recordSize]; memcpy(content, data, recordSize); }
    RM_Record(const RM_Record &rm_rec);
    RM_Record & operator=(const RM_Record &rm_rec);
    ~RM_Record() { if(content) delete [] content; }
    char * GetContent()const { return content; }
    RID GetRid() const { return rid; }

};

class RM_PageHandler {
    friend class RM_FileHandler;
private:
    PageHandler pf_pageHandler;
    RM_PageHeader rm_pageHeader;
    int recordSize;
    int nMaxRecordPerPage;
    char *pData;
public:
    RM_PageHandler() : recordSize(-1) {}
    RM_PageHandler(PageHandler &ph, RM_FileHeader rm_fileHeader);
    ~RM_PageHandler() {}

    bool HasFreeSpace() { return (rm_pageHeader.firstFreeRec != RECORD_FREE_LIST_END); }
    void InitPage();    //初始化新分配的Page(还没有存放记录),将页头和free list初始化
    char *GetDataPtr() { return pData; }
    int GetPageNum() { return pf_pageHandler.GetPageNum(); }
    int GetNextFreeSlot(int slot);

    RC GetRecord(int slot, RM_Record &rec);
    RC InsertRecord(char *pData, RID &rid);
    RC DeleteRecord(int slot);
    RC UpdataRecord(char *pData, RID &rid);

    //inline functions
    char *GetRecordAddr(int slot) { return &pData[slot *(recordSize + 1) + 1]; }
    bool isRecordInUse(int slot) { return pData[slot *(recordSize + 1)] == SLOT_IN_USE; }
    void MarkSlotInUse(int slot) { pData[slot *(recordSize + 1)] = SLOT_IN_USE; }
    void MarKSlotFree(int slot) { pData[slot *(recordSize + 1)] = SLOT_FREE; }
};

class RM_FileHandler {
    friend class RM_Manager;
private:
    int fd;
    bool bHeaderModified;
    RM_FileHeader rm_fh;
    FileHandler pf_fileHandler;

public:
    RM_FileHandler() : bHeaderModified(false) ,pf_fileHandler(), fd(-1){};
    ~RM_FileHandler() {};
    RC GetRec(const RID &rid, RM_Record &rec) const;
    RC InsertRec(char *pData, RID &rid);
    RC UpdateRec(const RM_Record &rec);
    RC DeleteRec(const RID &rid);

    void ForcePages(int pageNum = ALL_PAGES) const;
    RC AllocatePage(RM_PageHandler &rm_pageHandler);
    RC GetThisPage(int pageNum, RM_PageHandler &rm_pageHandler);
    void MarkDirty(int pageNum) { pf_fileHandler.MarkDirty(pageNum); }

    //getter and setter
    int getRecordSize() const { return rm_fh.RecordSize; }
    int getNMaxRecordPerPage() const { return rm_fh.nMaxRecordsPerPage; }
};


class RM_Manager {
private:
    const int MAX_RECORDSIZE  = (PAGE_SIZE - sizeof(pageHeader) - sizeof(RM_PageHeader));   //最大记录大小
    PF_Manager &pfm;

public:
    RM_Manager(PF_Manager &pfm) : pfm(pfm) { }
    ~RM_Manager() {};
    void CreateFile(string filename, int recordSize);   //recordSize最小为4个字节, 最大为MAX_RECORDSIZE个字节
    void OpenFile(string filename, RM_FileHandler &fileHandler);
    void CloseFile(RM_FileHandler &rm_fileHandler);
    void DestroyFile(string filename);
};


#endif //MINISQLFROM0_RM_MANAGER_H
