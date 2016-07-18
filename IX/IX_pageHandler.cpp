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
//    char *pData = GetDataPtr();
//    if(ix_pageHeader.pageType == LeafPage) {
//        RID rid;
//        for(int i = 0; i < ix_pageHeader.nCurPtrs; i++) {
//            Key k((&pData[i * leaf_entry_size]), attrType, attrLength);
//            rid = *reinterpret_cast<RID *>(&pData[i *(1 + attrLength + sizeof(RID)) + (1 + attrLength)]);
//            pair<Key, RID> p(k, rid);
//            leafNode.entries.push_back(p);
//
//        }
//    } else if(ix_pageHeader.pageType == InteriorPage) {
//        for(int i = 0; i < ix_pageHeader.nCurPtrs; i++) {
//            PagePtr pp = *(PagePtr *)(&pData[i * (1 + sizeof(PagePtr) + attrLength)]);
//            interNode.pointers.push_back(pp);
//            if(i < ix_pageHeader.nCurPtrs - 1) {
//                Key k((&pData[i * (1 + sizeof(PagePtr) + attrLength) + (1 + sizeof(PagePtr))]), attrType, attrLength);
//                interNode.keys.push_back(k);
//            }
//        }
//    }
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
//    vector< pair<Key, RID>>::iterator iter;
//    if(leafNode.entries.empty()) {  //如果leafNode为空
//        pair<Key, RID> entry(key, rid);
//        leafNode.entries.push_back(entry);
//        return;
//    }
//    if(key < leafNode.entries[0].first) {   //如果小于第一个entry
//        pair<Key, RID> entry(key, rid);
//        leafNode.entries.insert(leafNode.entries.begin(), entry);
//        return;
//    }
//    for(iter = leafNode.entries.begin(); iter != leafNode.entries.end(); iter++) {
//        if( key >= (*iter).first ) {
//                pair<Key, RID> entry(key, rid);
//                leafNode.entries.insert(++iter, entry);
//                return;
//        }
//    }
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
//    if(interNode.keys.empty()) {
//        interNode.keys.push_back(key);
//        interNode.pointers.push_back(page);
//        return;
//    }
//    vector<Key>::iterator iter;
//    int i = 0;
//    for(iter = interNode.keys.begin(); iter != interNode.keys.end(); iter++, i++) {
//        if( key > *iter) {
//            if(key == *iter) {
//                //相等的情况单独考虑
//            } else {
//                interNode.keys.insert(++iter, key);
//            }
//        }
//    }
//    vector<PagePtr>::iterator pp_iter;
//    int j = 0;
//    for(pp_iter = interNode.pointers.begin(); pp_iter != interNode.pointers.end(); pp_iter++, j++) {
//        if(j == i + 1) {
//            interNode.pointers.insert(++pp_iter, page);
//        }
//    }
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
