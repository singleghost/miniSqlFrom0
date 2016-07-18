//
// Created by 谢俊东 on 16/7/17.
//
#include "ix.h"

IX_PageHandler::IX_PageHandler(const IX_PageHandler &ix_pageHandler) : interior_entry_size(ix_pageHandler.interior_entry_size),
leaf_entry_size(ix_pageHandler.leaf_entry_size){
    this->ix_pageHeader = ix_pageHandler.ix_pageHeader;
    this->attrLength = ix_pageHandler.attrLength;
    this->attrType = ix_pageHandler.attrType;
    this->nMaxPtrInteriorPage = ix_pageHandler.nMaxPtrInteriorPage;
    this->nMaxPtrLeafPage = ix_pageHandler.nMaxPtrLeafPage;
    this->pf_pageHandler = ix_pageHandler.pf_pageHandler;
}
IX_PageHandler::IX_PageHandler(const PageHandler &pf_pageHandler, AttrType attrType, int attrLength) :
leaf_entry_size(attrLength + sizeof(RID)), interior_entry_size(sizeof(PagePtr) + attrLength)
{
    this->pf_pageHandler = pf_pageHandler;
    this->attrType = attrType;
    this->attrLength = attrLength;
    this->nMaxPtrLeafPage = (PAGE_SIZE - sizeof(pageHeader) - sizeof(IX_pageHeader))/ (attrLength + sizeof(RID)) - 1;//故意减一,留出一个空位,简化算法
    this->nMaxPtrInteriorPage = (PAGE_SIZE - sizeof(pageHeader) - sizeof(IX_pageHeader)) / (sizeof(PagePtr) + attrLength) - 1;//故意减一,留出一个空位,简化算法

    memcpy(&this->ix_pageHeader, this->pf_pageHandler.GetDataPtr(), sizeof(IX_pageHeader));
}

void IX_PageHandler::InsertLeafEntry(const Key &key, const RID &rid) {
    if(ix_pageHeader.nCurPtrs == 0){
        InsertLeafEntryToLoc(0, key, rid);
    }

    if(key < GetLeafKey(0)) {
        InsertLeafEntryToLoc(0, key, rid);
    }
    for(int i = 0; i < ix_pageHeader.nCurPtrs; i++) {
        if(key >= GetLeafKey(i)) {
            InsertLeafEntryToLoc(i + 1, key, rid);
        }
    }
}

void IX_PageHandler::InsertInteriorEntry(const Key &key, PagePtr page) {
    if(ix_pageHeader.nCurPtrs == 0) {
        InsertInterEntryToloc(0, key, page);
        return;
    }
    if(key < GetInteriorKey(0)) {
        InsertInterEntryToloc(0, key, page);
        return;
    }
    for(int i = 0; i < ix_pageHeader.nCurPtrs; i++) {
        if(key > GetInteriorKey(i)) {
            InsertInterEntryToloc(i + 1, key, page);
            return;
        }
        //相等的情况不考虑,暂时只实现unique属性的索引
    }
}
void IX_PageHandler::InitPage(PagePtr parentNode, PagePtr nextNode, IX_pageType pageType){
    this->ix_pageHeader.nCurPtrs = 0;
    this->ix_pageHeader.parentNode = parentNode;
    this->ix_pageHeader.nextNode = nextNode;
    this->ix_pageHeader.pageType = pageType;
}

void IX_PageHandler::InsertLeafEntryToLoc(int loc, const Key &key, const RID &rid) {
    if(loc < ix_pageHeader.nCurPtrs) {
        memmove(GetDataPtr() + (loc + 1) *leaf_entry_size, GetDataPtr() + loc * leaf_entry_size,
                 (ix_pageHeader.nCurPtrs - loc) * leaf_entry_size);
    }
    memcpy(GetDataPtr() + loc *leaf_entry_size, key.GetPtr(), attrLength);
    memcpy(GetDataPtr() + loc * leaf_entry_size + attrLength, &rid, sizeof(RID));
    ix_pageHeader.nCurPtrs++;
}

void IX_PageHandler::InsertInterEntryToloc(int loc, const Key &key, PagePtr page){
    if(loc < ix_pageHeader.nCurPtrs - 1) {
        memmove(GetDataPtr() + sizeof(PagePtr)+ (loc + 1) * interior_entry_size, GetDataPtr() + sizeof(PagePtr) + loc * interior_entry_size,
                ((ix_pageHeader).nCurPtrs - 1 - loc) * interior_entry_size);
    }
    memcpy(GetDataPtr() + sizeof(PagePtr) + loc *(attrLength + sizeof(PagePtr)), key.GetPtr(), attrLength );
    memcpy(GetDataPtr() + sizeof(PagePtr) + loc *(attrLength + sizeof(PagePtr) + attrLength), &page, sizeof(PagePtr));
    ix_pageHeader.nCurPtrs++;
}
