//
// Created by 谢俊东 on 16/7/17.
//
#include "ix.h"

IX_PageHandler::IX_PageHandler(const PageHandler &pf_pageHandler, AttrType attrType, int attrLength) {
    this->pf_pageHandler = pf_pageHandler;
    memcpy(&this->ix_pageHeader, this->pf_pageHandler.GetDataPtr(), sizeof(IX_pageHeader));

    char *pData = GetDataPtr();
    if(ix_pageHeader.pageType == LeafPage) {
        RID rid;
        for(int i = 0; i < ix_pageHeader.n_curPtrs; i++) {
            Key k((&pData[i * (1 + attrLength + sizeof(RID))]), attrType, attrLength);
            rid = *reinterpret_cast<RID *>(&pData[i *(1 + attrLength + sizeof(RID)) + (1 + attrLength)]);
            pair<Key, RID> p(k, rid);
            leafNode.entries.push_back(p);

        }
    } else if(ix_pageHeader.pageType == InteriorPage) {
        for(int i = 0; i < ix_pageHeader.n_curPtrs; i++) {
            PagePointer pp = *(PagePointer *)(&pData[i * (1 + sizeof(PagePointer) + attrLength)]);
            internalNode.pointers.push_back(pp);
            if(i < ix_pageHeader.n_curPtrs - 1) {
                Key k((&pData[i * (1 + sizeof(PagePointer) + attrLength) + (1 + sizeof(PagePointer))]), attrType, attrLength);
                internalNode.keys.push_back(k);
            }
        }
    }
}

void IX_PageHandler::InsertLeafEntry(const Key &key, const RID &rid) {
    vector< pair<Key, RID>>::iterator iter;
    for(iter = leafNode.entries.begin(); iter != leafNode.entries.end(); iter++) {
        if( key >= (*iter).first ) {
            pair<Key, RID> entry(key, rid);
            leafNode.entries.insert(iter, entry);
            break;
        }
    }
}
