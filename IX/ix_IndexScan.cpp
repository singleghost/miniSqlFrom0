//
// Created by 谢俊东 on 16/7/18.
//
#include <cassert>
#include "ix.h"

RC IX_IndexScan::OpenScan(const IX_IndexHandler &indexHandler, CompOp compOp, void *value, ClientHint pinHint) {
    assert(compOp != NO_OP && compOp != NE_OP);
    this->indexHandler = indexHandler;
    this->attrLength = indexHandler.ix_fh.attrLength;
    this->attrType = indexHandler.ix_fh.attrType;
    this->compOp = compOp;
    this->value = value;
    this->key = Key((char *) value, this->attrType, this->attrLength);
    this->bScanIsOpen = true;
    this->curPage = -1;
    this->curLoc = -1;
    this->pinHint = pinHint;
    return 0;
}

RC IX_IndexScan::GetNextEntry(RID &rid) {
    if (!this->bScanIsOpen) return -1;
    if(indexHandler.ix_fh.rootNode == NO_ROOT_NODE) return IX_EOF;
    IX_PageHandler ix_pageHandler;
    if (curPage == -1 && curLoc == -1) {
        curPage = indexHandler.Search(indexHandler.ix_fh.rootNode, this->key);
        if (indexHandler.GetThisPage(curPage, ix_pageHandler) == PF_PAGE_NOT_IN_USE) return PF_PAGE_NOT_IN_USE;
        if (ix_pageHandler.Get_Loc_From_Key(this->compOp, this->key, curLoc) == IX_KEY_NOT_FOUND)
            return IX_EOF;
        else {
            switch (compOp) {
                case EQ_OP: rid = ix_pageHandler.GetLeafRID(curLoc);
                    return 0;
                case GT_OP:
                case LT_OP:
                case GE_OP:
                case LE_OP:
                    break;
            }
        }
    } else {
        if (compOp == EQ_OP) return IX_EOF;
        if (indexHandler.GetThisPage(curPage, ix_pageHandler) == PF_PAGE_NOT_IN_USE) return PF_PAGE_NOT_IN_USE;
    }

    switch (compOp) {
        case GT_OP:
        case GE_OP:
            if (curLoc < ix_pageHandler.ix_pageHeader.nCurPtrs) rid = ix_pageHandler.GetLeafRID(curLoc);
            else {
                curPage = ix_pageHandler.ix_pageHeader.nextNode;
                if (curPage == -1) return IX_EOF;
                curLoc = 0;
                if (indexHandler.GetThisPage(curPage, ix_pageHandler) == PF_PAGE_NOT_IN_USE) return PF_PAGE_NOT_IN_USE;
                assert(curLoc < ix_pageHandler.ix_pageHeader.nCurPtrs);
                rid = ix_pageHandler.GetLeafRID(curLoc);
            }
            curLoc++;
            break;
        case LT_OP:
        case LE_OP:
            if (curLoc >= 0) rid = ix_pageHandler.GetLeafRID(curLoc);
            else {
                curPage = ix_pageHandler.ix_pageHeader.prevNode;
                if (curPage == -1) return IX_EOF;
                if (indexHandler.GetThisPage(curPage, ix_pageHandler) == PF_PAGE_NOT_IN_USE) return PF_PAGE_NOT_IN_USE;
                curLoc = ix_pageHandler.ix_pageHeader.nCurPtrs - 1;
                assert(curLoc >= 0);
                rid = ix_pageHandler.GetLeafRID(curLoc);
            }
            curLoc--;
            break;
    }
    return 0;
}

RC IX_IndexScan::CloseScan() {
    this->bScanIsOpen = false;
    return 0;
}

