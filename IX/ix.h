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

typedef int PagePtr;    //页指针

#define NO_ROOT_NODE -1
#define NO_PARENT_NODE -1   //没有父节点
#define NO_PREV_NODE -2
#define NO_NEXT_NODE -2
#define IX_PREV_LIST_END -1
#define IX_NEXT_LIST_END -1
//返回码
#define IX_PAGE_NOT_IN_USE -1
#define IX_NO_SUCH_ENTRY -2
#define IX_NO_MORE_ENTRY -3
#define IX_KEY_NOT_FOUND -4

//数据结构
struct IX_fileHeader {
    int IndexNo;    //SM模块保存Index信息数组的下标,里面会有index的属性名等等
    AttrType attrType;  //属性类型
    int attrLength;     //属性长度
    int rootNode;   //根结点
    int nMaxPtrInteriorPage;    //interior page最多能放多少指针
    int nMaxPtrLeafPage;        //leaf page最多能放多少指针
};

struct IX_pageHeader {
    int nCurPtrs;  //当前页存储了多少指针
    IX_pageType pageType;   //页类型,leaf page或者interior page
    PagePtr parentNode; //指向父节点
    PagePtr nextNode;   //pageType为leaf时此字段有效,指向链表中的下一个leaf page, -1表示链表结尾
    PagePtr prevNode;   //pageType为leaf时此字段有效,
};

//存放每个Entry的值
class Key {
private:
    char *ptr;          //存放指针
    AttrType attrType;
    int attrLength;
public:
    Key() {ptr = NULL;}
    Key(char *ptr, AttrType attrType , int attrLength) : attrLength(attrLength), attrType(attrType)
    { this->ptr = new char[attrLength]; memcpy(this->ptr, ptr, attrLength); }
    Key(const Key &key);
    Key &operator=(const Key &key);
    ~Key() { if(ptr) delete [] this->ptr; }
    char *GetPtr() const { return ptr; }    //获取指针
    //重载比较运算符
    bool operator==(const Key &key1) const;
    bool operator>(const Key &key1) const;
    bool operator<(const Key &key1) const;
    bool operator>=(const Key &key1) const;
    bool operator<=(const Key &key1) const;
    bool operator!=(const Key &key1) const;
};


class IX_PageHandler {
    friend class IX_IndexHandler;
    friend class IX_IndexScan;
private:
    int leaf_entry_size;            //页节点一个entry大小(包括属性长度,RID长度)
    int interior_entry_size;        //内部节点一个entry大小(包括属性长度,PagePtr长度)
    PageHandler pf_pageHandler;     //封装PF模块的PageHandler
    IX_pageHeader ix_pageHeader;    //IX模块的页头
    AttrType attrType;              //属性类型
    int attrLength;                 //属性长度
    int nMaxPtrInteriorPage;    //interior page最多能放多少指针(可以不用满整个Page)
    int nMaxPtrLeafPage;        //leaf page最多能放多少指针(可以不用满整个Page)
    //如果是内部节点

    void InitPage(PagePtr parentNode, PagePtr nextNode, PagePtr prevNode, IX_pageType pageType); //初始化Page
    //Getters
    int GetPageNum() { return pf_pageHandler.GetPageNum(); }    //获取当前页的页号
    int GetParentNode() { return ix_pageHeader.parentNode; }    //获取父页的页号
    int GetNextNode() { return ix_pageHeader.nextNode; }
    int GetPrevNode() { return ix_pageHeader.prevNode; }
    int GetPageType() { return ix_pageHeader.pageType; }
    int GetnCurPtr() { return ix_pageHeader.nCurPtrs; }          //获取当前的指针数
    Key GetLeafKey(int loc) { return Key(GetDataPtr() + loc * leaf_entry_size, attrType, attrLength); } //leaf page时根据loc获取Key值
    RID GetLeafRID(int loc) { return *(RID *)(GetDataPtr() + loc * leaf_entry_size + attrLength); }     //leaf page时根据loc获取RID值
    Key GetInteriorKey(int loc){ return Key(GetDataPtr() + loc * interior_entry_size + sizeof(PagePtr), attrType, attrLength); } //interior page时根据location获取Key值
    PagePtr GetInteriorPtr(int loc) { return *(PagePtr *)(GetDataPtr() + loc * interior_entry_size); }    //interior page时根据loc获取PagePtr

    //setters
    void increaseCurPtr() { ix_pageHeader.nCurPtrs++; memcpy(pf_pageHandler.GetDataPtr(), &ix_pageHeader, sizeof(IX_pageHeader)); }
    void setCurPtr(int num) { ix_pageHeader.nCurPtrs = num; memcpy(pf_pageHandler.GetDataPtr(), &ix_pageHeader, sizeof(IX_pageHeader));  }
    void setParentNode(int num) { ix_pageHeader.parentNode = num; memcpy(pf_pageHandler.GetDataPtr(), &ix_pageHeader, sizeof(IX_pageHeader)); }
    void setNextNode(int num)   { ix_pageHeader.nextNode = num; memcpy(pf_pageHandler.GetDataPtr(), &ix_pageHeader, sizeof(IX_pageHeader)); }
    void setPrevNode(int num)   { ix_pageHeader.prevNode = num; memcpy(pf_pageHandler.GetDataPtr(), &ix_pageHeader, sizeof(IX_pageHeader)); }

    void InsertLeafEntryToLoc(int loc, const Key &key, const RID &rid); //向leaf page中插入leaf entry
    void InsertInterEntryToloc(int loc, const Key &key, PagePtr page);  //向interior page中插入interior entry
    RC DeleteLeafEntry(const Key &key);     //根据key值删除叶节点中的某个entry
    RC DeleteInteriorEntry(const Key &key); //根据key值删除内部节点中的某个entry
    RC Get_Loc_From_Key(CompOp compOp, Key &key, int &loc);  //leaf page中根据key值,返回相应的rid和location


public:
    IX_PageHandler() : leaf_entry_size(0), interior_entry_size(0){}
    ~IX_PageHandler() {}
    IX_PageHandler(const PageHandler &pf_pageHandler, AttrType attrType, int attrLength);   //构造函数
    IX_PageHandler(const IX_PageHandler &ix_pageHandler);   //复制构造函数
    char *GetDataPtr() { return pf_pageHandler.GetDataPtr() + sizeof(IX_pageHeader); }      //获取指向数据的指针(不包括页头)
    void InsertLeafEntry(const Key &key, const RID &rid);   //向leaf page中插入一个entry
    void InsertInteriorEntry(const Key &key, PagePtr page); //向interior page中插入一个entry

    //调试用
    void PrintLeafEntries();
    void PrintInteriorEntries();
};

class IX_IndexHandler {
    friend class IX_Manager;
    friend class IX_IndexScan;
private:
    IX_fileHeader ix_fh;        //IX模块的文件头
    FileHandler pf_fileHandler; //封装PF模块的FileHandler
    int fd;     //文件描述符
    bool bHeaderModified;   //文件头是否被改变

    void setRootNode(int rootNode) { ix_fh.rootNode = rootNode; bHeaderModified = true; }
    int Search(int page, const Key &key);      //从某页开始搜索某个值,返回该值所在的pageNum(递归调用函数)
    int Insert_into_Leaf(int target_page, const Key &key, const RID &rid);  //向target leaf page插入key和rid
    int Insert_into_Interior(int target_page, const Key &key, PagePtr pPage);//向target interior page插入key和PagePtr
    int Delete_from_Interior(int target_page, PagePtr child_page);          //根据child page从target interior page中删除entry

    RC GetThisPage(int pageNum, IX_PageHandler &ix_pageHandler);    //获取指定的某一页,记得UnpinPage!
    RC AllocatePage(IX_PageHandler &ix_pageHandler);    //记得UnpinPage!
    RC DisposePage(int pageNum) { return pf_fileHandler.DisposePage(pageNum); }
    RC MarkDirty(int pageNum) { return pf_fileHandler.MarkDirty(pageNum); }
    void UnpinPage(int pageNum) { pf_fileHandler.UnpinPage(pageNum); }
public:
    IX_IndexHandler() {}
    ~IX_IndexHandler() {}
    RC InsertEntry(void *pData, const RID &rid);        //B+树的插入操作
    RC DeleteEntry(void *pData, const RID &rid);        //B+树的删除操作
    RC ForcePages() { return pf_fileHandler.ForcePages(); }   //将全部Dirty Page写回磁盘

    //调试用
    void PrintAll();
    void PrintInterNode(int node);
};

class IX_Manager {
private:
    PF_Manager & pfm;

public:
    IX_Manager(PF_Manager &pfm) :pfm(pfm) {}
    ~IX_Manager() {}
    RC CreateIndex(string filename, int indexNo, AttrType attrType, int attrLength);    //创建索引, 索引文件名为"filename.indexNo"
    RC DestroyIndex(string filename, int indexNo);  //删除索引, filename 和索引依赖的data file同名, 加上indexNo唯一标识索引
    RC OpenIndex(string filename, int indexNo, IX_IndexHandler &ix_handler);        //打开索引
    RC CloseIndex(IX_IndexHandler &ix_handler); //关闭索引
};


class IX_IndexScan {
private:
    IX_IndexHandler indexHandler;
    AttrType attrType;
    int attrLength;
    CompOp compOp;  //比较运算符
    void *value;    //比较的值
    Key key;
    ClientHint pinHint; //pin策略
    int curPage;        //当前scan到的page
    int curLoc;         //当前scan到的Loction
    bool bScanIsOpen;
public:
    IX_IndexScan() {}
    ~IX_IndexScan() {}
    RC OpenScan(const IX_IndexHandler &indexHandler, CompOp compOp, void *value, ClientHint pinHint=NO_HINT);   //打开并初始化一个扫描
    RC GetNextEntry(RID &rid);  //获取下一个Entry的rid, 没有下一个时返回IX_NO_MORE_ENTRY
    RC CloseScan(); //关闭扫描
};
#endif //MINISQLFROM0_IX_H
