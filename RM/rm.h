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
#define RM_EOF -2
#define RM_PAGE_RECORD_EOF -3

struct RM_PageHeader {
    int firstFreeRec;   //第一个空闲的record的位置
    int NumOfRecords;   //当前页共有多少Record, 不包括空闲的
};  //RM模块的页头

struct RM_FileHeader {
    int RecordSize;         //每条记录的长度
    int nMaxRecordsPerPage; //一页最多能存储多少条记录
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

    //getter
    char * GetContent()const { return content; }
    RID GetRid() const { return rid; }
    int GetRecordSize() const { return recordSize; }
};

class RM_PageHandler {
    friend class RM_FileHandler;
    friend class RM_FileScan;
private:
    PageHandler pf_pageHandler; //封装了底层的PF模块的PageHandler
    RM_PageHeader rm_pageHeader;    //RM模块的页头
    int recordSize;                 //记录的大小
    int nMaxRecordPerPage;          //每页最多能放多少条记录
    char *pData;                    //指向记录开始的地方(不包括RM模块的页头)

    //inline functions
    char *GetRecordAddr(int slot) { return &pData[slot *(recordSize + 1) + 1]; }
    bool isRecordInUse(int slot) { return pData[slot *(recordSize + 1)] == SLOT_IN_USE; }   //判断某个slot是否被使用
    void MarkSlotInUse(int slot) { pData[slot *(recordSize + 1)] = SLOT_IN_USE; }           //将某个slot标志为被使用
    void MarKSlotFree(int slot) { pData[slot *(recordSize + 1)] = SLOT_FREE; }              //将某个slot标志为空闲
    bool HasFreeSpace() { return (rm_pageHeader.firstFreeRec != RECORD_FREE_LIST_END); }    //当前页是否还有空间存放记录
    void InitPage();    //初始化新分配的Page(还没有存放记录),将页头和free list初始化

    //getter
    char *GetDataPtr() { return pData; }
    int GetPageNum() { return pf_pageHandler.GetPageNum(); }
    //free list相关的操作
    int GetNextFreeSlot(int slot);  //获取下一个空闲的槽位
    RC GetNextRecord(int cur_slot, RM_Record &rec);
public:
    RM_PageHandler() : recordSize(-1) {}    //默认构造函数
    RM_PageHandler(PageHandler &ph, RM_FileHeader rm_fileHeader);
    ~RM_PageHandler() {}

    RC GetRecord(int slot, RM_Record &rec); //根据slot获取一条记录,存放在rec中
    RC InsertRecord(char *pData, RID &rid); //插入一条记录,记录内容为pData所指向的内容,插入的位置存放在rid中
    RC DeleteRecord(int slot);              //删除一条记录
    RC UpdateRecord(char *pData, RID &rid); //更新一条记录,pData指定更新的内容, rid指定更新的记录id

};

class RM_FileHandler {
    friend class RM_Manager;
    friend class RM_FileScan;
private:
    int fd; //文件描述符
    bool bHeaderModified;   //RM模块的文件头是否被修改
    RM_FileHeader rm_fh;    //RM模块的文件头
    FileHandler pf_fileHandler; //封装了PF模块的FileHandler

    RC AllocatePage(RM_PageHandler &rm_pageHandler);    //分配一个Page
    RC DisposePage(int pageNum);                        //释放一个Page, 加入page free list中
    RC GetThisPage(int pageNum, RM_PageHandler &rm_pageHandler) const;  //得到某一个Page
    RC GetNextPage(int pageNum, RM_PageHandler &rm_pageHandler) const;  //得到下一个Page
    void MarkDirty(int pageNum) { pf_fileHandler.MarkDirty(pageNum); }
    void UnpinPage(int pageNum)const;

public:
    RM_FileHandler() {}
    ~RM_FileHandler() {};
    RC GetRec(const RID &rid, RM_Record &rec) const;    //根据rid获取一条记录,结果存放在rec中
    RC InsertRec(char *pData, RID &rid);          //插入一条记录,插入的位置存放在rid中
    RC UpdateRec(const RM_Record &rec);           //用rec来更新一条记录
    RC DeleteRec(const RID &rid);                 //根据rid来删除一条记录

    void ForcePages(int pageNum = ALL_PAGES) const;     //将Page写回磁盘,默认写回所有dirty page

    //getter and setter 测试用
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
    RC CreateFile(string filename, int recordSize);   //recordSize最小为4个字节, 最大为MAX_RECORDSIZE个字节
    RC OpenFile(string filename, RM_FileHandler &fileHandler);  //打开文件,fileHandler成为文件的句柄
    RC CloseFile(RM_FileHandler &rm_fileHandler);               //关闭文件
    RC DestroyFile(string filename);                            //删除文件
};


class RM_FileScan {

private:
    RM_FileHandler rm_fileHandler;
    AttrType attrType;
    int attrLength;
    int attrOffset;
    CompOp compOp;
    void *value;
    ClientHint pinHint;

    RID cur_rid;    //当前Scan到的record的RID
    bool bScanIsOpen;
    bool (*comparator) (const void * , const void *, AttrType, int);

public:
    RM_FileScan() {}
    ~RM_FileScan() {}
    RC OpenScan(const RM_FileHandler &rm_fileHandler,
                AttrType attrType,
                int attrLength,
                int attrOffset,
                CompOp compOp,
                void *value,
                ClientHint pinHint=NO_HINT);
    RC GetNextRec(RM_Record &rec);  //得到下一个匹配的record,当没有record符合条件时返回RM_EOF
    RC CloseScan();                 //关闭scan
};

//一些全局函数
namespace myComp {
    bool less_than(const void *value1, const void *value2, AttrType attrType, int attrLength);

    bool less_than_or_equal(const void *value1, const void *value2, AttrType attrType, int attrLength);

    bool greater_than(const void *value1, const void *value2, AttrType attrType, int attrLength);

    bool greater_than_or_equal(const void *value1, const void *value2, AttrType attrType, int attrLength);

    bool equal_To(const void *value1, const void *value2, AttrType attrType, int attrLength);

    bool not_equal_to(const void *value1, const void *value2, AttrType attrType, int attrLength);
}
#endif //MINISQLFROM0_RM_MANAGER_H
