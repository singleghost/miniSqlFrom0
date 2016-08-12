//
// Created by 谢俊东 on 16/7/14.
//

#include "rm.h"

RC RM_FileHandler::GetRec(const RID &rid, RM_Record &rec) const {
    RM_PageHandler rm_pageHandler;
    int page = rid.getPageNum();
    GetThisPage(page, rm_pageHandler);
    if(rm_pageHandler.GetRecord(rid.getSlot(), rec) == RECORD_NOT_IN_USE ) {
        UnpinPage(page);
        return RECORD_NOT_IN_USE;
    }
    UnpinPage(page);    //与GetTHisPage相对应
    return 0;
}

RC RM_FileHandler::InsertRec(char *pData, RID &rid) {
    PageHandler pf_pageHandler;
    RM_PageHandler rm_pageHandler;
    int page = -1;
    bool hasNextPage = true;
    while(hasNextPage) {
        try {
            GetNextPage(page, rm_pageHandler);
        } catch (page_not_found_exception) {
            hasNextPage = false;
            continue;
        }
        if(rm_pageHandler.HasFreeSpace()) {
            rm_pageHandler.InsertRecord(pData, rid);
            UnpinPage(rm_pageHandler.GetPageNum());
            MarkDirty(rm_pageHandler.GetPageNum());
            return 0;
        }
        page = rm_pageHandler.GetPageNum();
        UnpinPage(page);
    }
    //如果所有Page都没有空间插入了

    AllocatePage(rm_pageHandler);
    rm_pageHandler.InitPage();
    rm_pageHandler.InsertRecord(pData, rid);

    UnpinPage(rm_pageHandler.GetPageNum());
    MarkDirty(rm_pageHandler.GetPageNum());
    return 0;
}


RC RM_FileHandler::UpdateRec(const RM_Record &rec) {
    char *pNewRec = rec.GetContent();
    RID rid = rec.GetRid();
    int page = rid.getPageNum();
    RM_PageHandler rm_pageHandler;
    if( GetThisPage(page, rm_pageHandler) == PF_PAGE_NOT_IN_USE) return PF_PAGE_NOT_IN_USE;
    rm_pageHandler.UpdateRecord(pNewRec, rid);

    MarkDirty(page);
    UnpinPage(page);
    return 0;
}

RC RM_FileHandler::DeleteRec(const RID &rid) {
    int page = rid.getPageNum();
    int slot = rid.getSlot();
    RM_PageHandler rm_pageHandler;
    if( GetThisPage(page, rm_pageHandler) == PF_PAGE_NOT_IN_USE) return PF_PAGE_NOT_IN_USE;
    rm_pageHandler.DeleteRecord(slot);
//    printf("%d\n", rm_pageHandler.rm_pageHeader.NumOfRecords);    调试用
    if(rm_pageHandler.rm_pageHeader.NumOfRecords == 0) {
        DisposePage(rm_pageHandler.GetPageNum());
    }
    MarkDirty(page);
    UnpinPage(page);
    return 0;
}

RC RM_FileHandler::AllocatePage(RM_PageHandler &rm_pageHandler) {
    PageHandler pf_pageHandler;
    pf_fileHandler.AllocatePage(pf_pageHandler);
    rm_pageHandler = RM_PageHandler(pf_pageHandler, rm_fh);

    rm_pageHandler.InitPage();
    return 0;
}

RC RM_FileHandler::GetThisPage(int pageNum, RM_PageHandler &rm_pageHandler) const{
    RC rc;
    PageHandler pf_pageHandler;
    if( (rc = pf_fileHandler.GetThisPage(pageNum, pf_pageHandler)))
        return rc;
    rm_pageHandler = RM_PageHandler(pf_pageHandler, rm_fh);

    return 0;
}

//将page写回磁盘,默认参数为-1,写回所有dirty page
void RM_FileHandler::ForcePages(int pageNum) const {
    pf_fileHandler.ForcePages(pageNum);
}

RC RM_FileHandler::GetNextPage(int pageNum, RM_PageHandler &rm_pageHandler) const{
    PageHandler pf_pageHandler;
    pf_fileHandler.GetNextPage(pageNum, pf_pageHandler);
    rm_pageHandler = RM_PageHandler(pf_pageHandler, rm_fh);
    return 0;
}

//释放某一页
RC RM_FileHandler::DisposePage(int pageNum) {
    pf_fileHandler.DisposePage(pageNum);
    return 0;
}

void RM_FileHandler::UnpinPage(int pageNum)const {
    pf_fileHandler.UnpinPage(pageNum);
}