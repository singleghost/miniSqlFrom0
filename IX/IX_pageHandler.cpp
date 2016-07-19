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
    if(ix_pageHeader.nCurPtrs == 0){    //如果为空
        InsertLeafEntryToLoc(0, key, rid);
        return;
    }

    if(key < GetLeafKey(0)) {   //小于第一个
        InsertLeafEntryToLoc(0, key, rid);
        return;
    }
    if(key >= GetLeafKey(GetnCurPtr() - 1)) {   //大于最后一个
        InsertLeafEntryToLoc(GetnCurPtr(), key, rid);
        return;
    }
    for(int i = 0; i < GetnCurPtr() - 1; i++) {
        if(key >= GetLeafKey(i) && key < GetLeafKey(i + 1)) {
            InsertLeafEntryToLoc(i + 1, key, rid);
            return;
        }
    }
}

void IX_PageHandler::InsertInteriorEntry(const Key &key, PagePtr page) {
    if(ix_pageHeader.nCurPtrs == 1) {   //如果为空(只有一个指针)
        InsertInterEntryToloc(0, key, page);
        return;
    }
    if(key < GetInteriorKey(0)) {   //小于第一个
        InsertInterEntryToloc(0, key, page);
        return;
    }
    if(key > GetInteriorKey(GetnCurPtr() - 2)) {    //大于最后一个
        InsertInterEntryToloc(GetnCurPtr() - 1, key, page);
        return;
    }
    for(int i = 0; i < GetnCurPtr() - 1 - 1; i++) {
        if(key >= GetInteriorKey(i) && key < GetInteriorKey(i + 1)) {
            InsertInterEntryToloc(i + 1, key, page);
            return;
        }
        //相等的情况不考虑,暂时只实现unique属性的索引
    }
}

void IX_PageHandler::InitPage(PagePtr parentNode, PagePtr nextNode, PagePtr prevNode, IX_pageType pageType){
    this->ix_pageHeader.nCurPtrs = 0;
    this->ix_pageHeader.parentNode = parentNode;
    this->ix_pageHeader.nextNode = nextNode;
    this->ix_pageHeader.prevNode = prevNode;
    this->ix_pageHeader.pageType = pageType;
    memcpy(pf_pageHandler.GetDataPtr(), &ix_pageHeader, sizeof(IX_pageHeader));
}

void IX_PageHandler::InsertLeafEntryToLoc(int loc, const Key &key, const RID &rid) {
    if(loc < ix_pageHeader.nCurPtrs) {
        memmove(GetDataPtr() + (loc + 1) *leaf_entry_size, GetDataPtr() + loc * leaf_entry_size,
                 (ix_pageHeader.nCurPtrs - loc) * leaf_entry_size);
    }
    memmove(GetDataPtr() + loc *leaf_entry_size, key.GetPtr(), attrLength);
    memmove(GetDataPtr() + loc * leaf_entry_size + attrLength, &rid, sizeof(RID));
    increaseCurPtr();
}

void IX_PageHandler::InsertInterEntryToloc(int loc, const Key &key, PagePtr page){
    if(loc < ix_pageHeader.nCurPtrs - 1) {
        memmove(GetDataPtr() + sizeof(PagePtr)+ (loc + 1) * interior_entry_size, GetDataPtr() + sizeof(PagePtr) + loc * interior_entry_size,
                ((ix_pageHeader).nCurPtrs - 1 - loc) * interior_entry_size);
    }
    memcpy(GetDataPtr() + sizeof(PagePtr) + loc *(attrLength + sizeof(PagePtr)), key.GetPtr(), attrLength );
    memcpy(GetDataPtr() + sizeof(PagePtr) + loc *(attrLength + sizeof(PagePtr)) + attrLength, &page, sizeof(PagePtr));
    increaseCurPtr();
}

RC IX_PageHandler::DeleteLeafEntry(const Key &key) {
    for(int i = 0; i < ix_pageHeader.nCurPtrs; i++) {
        if( key == GetLeafKey(i)) {
            memcpy(GetDataPtr() + i *(leaf_entry_size), GetDataPtr() + (i+1) *leaf_entry_size,
                   (ix_pageHeader.nCurPtrs - i - 1) * leaf_entry_size);
            setCurPtr(GetnCurPtr() - 1);
            return 0;
        }
    }
    return IX_NO_SUCH_ENTRY;
}

RC IX_PageHandler::Get_Loc_From_Key(CompOp compOp, Key &key, int &loc){
    switch (compOp) {
        case EQ_OP:
            for(int i = 0; i < ix_pageHeader.nCurPtrs; i++) {
                if(key == GetLeafKey(i)) {
                    loc = i;
                    return 0;
                }
            }
            break;
        case GT_OP:
            for(int i = 0; i < ix_pageHeader.nCurPtrs; i++) {
                if(key > GetLeafKey(i)) {
                    loc = i + 1;
                    return 0;
                }
            }
            break;
        case GE_OP:
            for(int i = 0; i < ix_pageHeader.nCurPtrs; i++) {
                if(key >= GetLeafKey(i)) {
                    loc = i;
                    return 0;
                }
            }
            break;
        case LT_OP:
            for(int i = 0; i < ix_pageHeader.nCurPtrs; i++) {
                if(key < GetLeafKey(i)) {
                    loc = i - 1;
                    return 0;
                }
            }
            break;
        case LE_OP:
            for(int i = 0; i < ix_pageHeader.nCurPtrs; i++) {
                if(key <= GetLeafKey(i)) {
                    loc = i;
                    return 0;
                }
            }
            break;
        default:
            break;
    }
    return IX_KEY_NOT_FOUND;
}

void IX_PageHandler::PrintLeafEntries() {
    printf("Leaf Page; PageNum: %d; parentNode: %d; nextNode: %d; prevNode: %d; nCurPtr: %d\n", GetPageNum(), GetParentNode(), ix_pageHeader.nextNode,
    ix_pageHeader.prevNode, GetnCurPtr());
    printf("key\tpage\tslot\n");
    for(int i = 0; i < GetnCurPtr(); i++) {
        printf("%3d\t%4d\t%4d\n", *(int *)GetLeafKey(i).GetPtr(), GetLeafRID(i).getPageNum(), GetLeafRID(i).getSlot());
    }
}

void IX_PageHandler::PrintInteriorEntries() {
    printf("Interior Page; PageNum: %d; parentNode: %d\n", GetPageNum(), GetParentNode());
    for(int i = 0; i < GetnCurPtr(); i++) {
        printf("%d | ", GetInteriorPtr(i));
        if(i < GetnCurPtr() - 1) printf("%d | ", *(int *)GetInteriorKey(i).GetPtr());
    }
    printf("\n");
}