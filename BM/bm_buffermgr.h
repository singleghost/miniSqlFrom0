//
// Created by root on 16-6-5.
//

#ifndef MINISQLFROM0_BM_BUFFERMGR_H
#define MINISQLFROM0_BM_BUFFERMGR_H

#include "bm_hashTable.h"
#include "../minisql.h"

struct bufferPageDesc{
    char *page_addr;    //指向Page的实际内容,大小为PAGE SIZE
    int next;           //链表的next指针
    int prev;           //链表的prev指针
    int pinCount;       //当前页的pin的次数
    bool isdirty;       //初始为false,如果当前页被修改过,isdirty为true
    int pageNum;        //当前页的页号
    int fd;             //当前页所属的文件描述符
};

class BM_BufferMgr {
private:
    bufferPageDesc *PageDescTable;  //描述符表,用来存放内存中所有页的描述符信息, 大小由numOfPages指定
    int firstFree;  //free list 的表头
    int firstUsed; //MRU (Most Recent Used Page) used list 的表头
    int lastUsed; //LRU (Least Recent Userd Page) used list 的表尾
    BM_HashTable *hashTable;    //哈希表,用来快速定位某个文件的某一页在Page Description Table中的位置

    int numOfPages;             //Page Description Table的大小
    const int pageSize = 4096;

public:
    BM_BufferMgr(int numOfPages);   //参数numOfPages指定Page Description Table的大小

    ~BM_BufferMgr();

    RC AllocatePage(int fd, int pageNum, char *&ppointer);  //从buffer pool中分配一个新的Empty Page,如果已存在则返回error
    //将Page所占的buffer pool的空间释放
    RC UnPinPage(int fd, int pageNum);                      //Page的pinCount减一
    void ForcePage(int fd, int pageNum);    //把一页写回硬盘,但不从buffer pool里移除
    void writeBackAllDirty(int fd);         //把某文件的所有dirty page写回磁盘, 但不从buffer pool里移除
    RC GetPage(int fd, int pageNum, char *&ppointer);     //返回Page Content在内存的地址,先在内存中找,如果找不到就从磁盘中读取
    void MarkDirty(int fd, int pageNum);    //标记Page为dirty

    //may be used for test only!
    void ReadPage(int fd, int pageNum, char *dest);     //从磁盘将文件的一个Page读入dest数组,dest数组大小至少为PAGE SIZE
    void WritePage(int fd, int pageNum , char *src);    //将src数组内PAGE SIZE个字节写入磁盘上的文件
    void PrintPageDescTable();                          //打印当前Page description Table的值
private:
    RC InternalAlloc(int &slot);        //找到一个可用的slot,优先从free list中找,没有的话就把unpin的Page替换出去
    RC InitPageDesc(int fd, int pageNum, int slot); //初始化Page Description项
    RC UsedLinkHead(int slot);  //将slot放在usedlist的头
    RC UnlinkFromFree(int slot);    //将slot从freelist中移除
    RC UnlinkFromUsed(int slot);    //将slot从usedlist中移除
    RC FreeLinkHead(int slot);  //将slot放在freelist的表头
};


#endif //MINISQLFROM0_BM_BUFFERMGR_H
