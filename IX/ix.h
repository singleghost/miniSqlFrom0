//
// Created by 谢俊东 on 16/7/16.
//

#ifndef MINISQLFROM0_IX_H
#define MINISQLFROM0_IX_H

#include <map>
#include <vector>
#include "../PF/pf_filemgr.h"
#include "../RM/rm.h"

using namespace std;

typedef int PagePtr;

#define NO_PARENT_NODE -1
//返回码
#define IX_PAGE_NOT_IN_USE -1

struct IX_fileHeader {
    int IndexNo;    //SM模块保存Index信息数组的下标,里面会有index的属性名等等
    AttrType attrType;
    int attrLength;
    int rootNode;   //根结点
    int nMaxPtrInteriorPage;    //interior page最多能放多少指针
    int nMaxPtrLeafPage;        //leaf page最多能放多少指针
};

struct IX_pageHeader {
    IX_pageType pageType;   //页类型,leaf page或者interior page
    int nCurPtrs;  //当前页存储了多少指针
    PagePtr parentNode; //指向父节点
    PagePtr nextNode;   //pageType为leaf时此字段有效,指向链表中的下一个leaf page, -1表示链表结尾
};
//数据结构

//存放每个Entry的值
class Key {
private:
    char *ptr;
    AttrType attrType;
    int attrLength;
public:
    Key(char *key, AttrType attrType , int attrLength) : attrLength(attrLength), attrType(attrType)
    { this->ptr = new char[attrLength]; memcpy(this->ptr, key, attrLength); }
    Key(const Key &key);
    Key &operator=(const Key &key);
    ~Key() { delete [] this->ptr; }
    char *GetPtr() const { return ptr; }
    //重载比较运算符
    bool operator==(const Key &key1) const;
    bool operator>(const Key &key1) const;
    bool operator<(const Key &key1) const;
    bool operator>=(const Key &key1) const;
    bool operator<=(const Key &key1) const;
    bool operator!=(const Key &key1) const;
};
//叶节点
struct LeafNode {
    vector< pair<Key, RID>> entries;
//    PagePtr nextPage;
};

//内部节点
struct InternalNode {
    vector<Key> keys;   //值
    vector<PagePtr> pointers;   //指针
};


class IX_PageHandler {
    friend class IX_IndexHandler;
private:
    int leaf_entry_size;
    int interior_entry_size;
    PageHandler pf_pageHandler;     //封装PF模块的PageHandler
    IX_pageHeader ix_pageHeader;    //IX模块的页头
//    LeafNode leafNode;              //叶节点,存放实际内容
//    InternalNode interNode;      //内部节点,存放实际内容
    AttrType attrType;              //属性类型
    int attrLength;                 //属性长度
    int nMaxPtrInteriorPage;    //interior page最多能放多少指针
    int nMaxPtrLeafPage;        //leaf page最多能放多少指针
    //如果是内部节点

    void InitPage(PagePtr parentNode, PagePtr nextNode, IX_pageType pageType);
    int GetPageNum() { return pf_pageHandler.GetPageNum(); }
    int GetParentNode() { return ix_pageHeader.parentNode; }
    Key GetLeafKey(int loc) { return Key(GetDataPtr() + loc * (1 + attrLength + sizeof(RID)) + 1, attrType, attrLength); }
    Key GetInteriorKey(int loc){ return Key(GetDataPtr() + loc * (1 + sizeof(PagePtr) + attrLength) + 1 + sizeof(PagePtr), attrType, attrLength); }
    PagePtr GetInteriorPtr(int loc) { return *(PagePtr *)(GetDataPtr() + loc *(1 + sizeof(PagePtr) + attrLength) + 1); }

    void InsertLeafEntryToLoc(int loc, const Key &key, const RID &rid);
    void InsertInterEntryToloc(int loc, const Key &key, PagePtr page);
public:
    IX_PageHandler() : leaf_entry_size(0), interior_entry_size(0){}
    ~IX_PageHandler() {}
    IX_PageHandler(const PageHandler &pf_pageHandler, AttrType attrType, int attrLength);   //构造函数
    IX_PageHandler(const IX_PageHandler &ix_pageHandler);   //复制构造函数
    char *GetDataPtr() { return pf_pageHandler.GetDataPtr() + sizeof(IX_pageHeader); }      //获取指向数据的指针(不包括页头)
    void InsertLeafEntry(const Key &key, const RID &rid);
    void InsertInteriorEntry(const Key &key, PagePtr page);
};

class IX_IndexHandler {
    friend class IX_Manager;
private:
    IX_fileHeader ix_fh;        //IX模块的文件头
    FileHandler pf_fileHandler; //封装PF模块的FileHandler
    int fd;     //文件描述符
    bool bHeaderModified;   //文件头是否被改变

    int Search(int page, Key key);      //从某页开始搜索某个值,返回该值所在的pageNum
    int Insert_into_Leaf(int page, const Key &key, const RID &rid);
    int Insert_into_interior(int target_page, const Key &key, PagePtr pPage);

    void WriteToBlock();
    RC GetThisPage(int pageNum, IX_PageHandler &ix_pageHandler);    //获取指定的某一页
    RC AllocatePage(IX_PageHandler &ix_pageHandler);
    RC MarkDirty(int pageNum) { return pf_fileHandler.MarkDirty(pageNum); }
    void UnpinPage(int pageNum) { pf_fileHandler.UnpinPage(pageNum); }
public:
    IX_IndexHandler() {}
    ~IX_IndexHandler() {}
    RC InsertEntry(void *pData, const RID &rid);        //B+树的插入操作
    RC DeleteEntry(void *pData, const RID &rid);        //B+树的删除操作
    RC ForcePages() { return pf_fileHandler.ForcePages(); }   //将全部Dirty Page写回磁盘
};

class IX_Manager {
private:
    PF_Manager & pfm;

public:
    IX_Manager(PF_Manager &pfm) :pfm(pfm) {}
    ~IX_Manager() {}
    RC CreateIndex(string filename, int indexNo, AttrType attrType, int attrLength);
    RC DestroyIndex(string filename, int indexNo);
    RC OpenIndex(string filename, int indexNo, IX_IndexHandler &ix_handler);
    RC CloseIndex(IX_IndexHandler &ix_handler);
};


class IX_IndexScan {
public:
    IX_IndexScan() {}
    ~IX_IndexScan() {}
    RC OpenScan(const IX_IndexHandler &indexHandler, CompOp compOp, void *value, ClientHint pinHint=NO_HINT);
    RC GetNextEntry(RID &rid);
    RC CloseScan();
};
#endif //MINISQLFROM0_IX_H
