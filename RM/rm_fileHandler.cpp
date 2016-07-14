//
// Created by 谢俊东 on 16/7/14.
//

#include "rm.h"

RC RM_FileHandler::GetRec(const RID &rid, RM_Record &rec) const {
    PageHandler pHandler;
    pf_fileHandler.GetThisPage(rid.getPageNum(), pHandler);
    RM_PageHandler rm_pageHandler(pHandler, this->rm_fh);
    if(rm_pageHandler.GetRecord(rid.getSlot(), rec) == RECORD_NOT_IN_USE ) return RECORD_NOT_IN_USE;
    return 0;
}

RC RM_FileHandler::InsertRec(char *pData, RID &rid) {
    PageHandler pf_pageHandler;
    RM_PageHandler rm_pageHandler;
    int page = -1;
    bool hasNextPage = true;
    while(hasNextPage) {
        try {
            pf_fileHandler.GetNextPage(page, pf_pageHandler);
        } catch (page_not_found_exception) {
            hasNextPage = false;
            continue;
        }
        rm_pageHandler = RM_PageHandler(pf_pageHandler, rm_fh);
        if(rm_pageHandler.HasFreeSpace()) {
            rm_pageHandler.InsertRecord(pData, rid);
        }
        page = pf_pageHandler.GetPageNum();
    }
    //如果所有Page都没有空间插入了

    AllocatePage(rm_pageHandler);
    rm_pageHandler.InsertRecord(pData, rid);
    MarkDirty(page);
    return 0;
}

RC RM_FileHandler::UpdateRec(const RM_Record &rec) {
    char *pNewRec = rec.GetContent();
    RID rid = rec.GetRid();
    int page = rid.getPageNum();
    int slot = rid.getSlot();
    RM_PageHandler rm_pageHandler;
    if( GetThisPage(page, rm_pageHandler) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    rm_pageHandler.UpdataRecord(pNewRec, rid);

    MarkDirty(page);
    return 0;
}

RC RM_FileHandler::DeleteRec(const RID &rid) {
    int page = rid.getPageNum();
    int slot = rid.getSlot();
    RM_PageHandler rm_pageHandler;
    if( GetThisPage(page, rm_pageHandler) == PAGE_NOT_IN_USE) return PAGE_NOT_IN_USE;
    rm_pageHandler.DeleteRecord(slot);
    MarkDirty(page);

    return 0;
}

RC RM_FileHandler::AllocatePage(RM_PageHandler &rm_pageHandler) {
    PageHandler pf_pageHandler;
    pf_fileHandler.AllocatePage(pf_pageHandler);
    rm_pageHandler = RM_PageHandler(pf_pageHandler, rm_fh);
    rm_pageHandler.InitPage();
    return 0;
}

RC RM_FileHandler::GetThisPage(int pageNum, RM_PageHandler &rm_pageHandler) {
    PageHandler pageHandler;
    if( pf_fileHandler.GetThisPage(pageNum, pageHandler) == PAGE_NOT_IN_USE)
        return PAGE_NOT_IN_USE;
    char *p = pageHandler.GetDataPtr();
    rm_pageHandler = RM_PageHandler(pageHandler, rm_fh);
    memcpy(&rm_pageHandler.rm_pageHeader, p, sizeof(pageHeader));

    return 0;
}

void RM_FileHandler::ForcePages(int pageNum) const {
    pf_fileHandler.ForcePages(pageNum);
}

