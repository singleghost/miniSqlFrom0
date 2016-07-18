//
// Created by 谢俊东 on 16/7/14.
//

#include <cassert>
#include "rm.h"

RM_PageHandler::RM_PageHandler(PageHandler &ph, RM_FileHeader rm_fileHeader){
    pf_pageHandler = ph;
    recordSize = rm_fileHeader.RecordSize;
    nMaxRecordPerPage = rm_fileHeader.nMaxRecordsPerPage;
    memcpy(&rm_pageHeader, ph.GetDataPtr(), sizeof(RM_PageHeader));
    pData = ph.GetDataPtr() + sizeof(RM_PageHeader);
}

RC RM_PageHandler::GetRecord(int slot, RM_Record &rec) {
    assert(slot < nMaxRecordPerPage);
    if(!isRecordInUse(slot))
        return RECORD_NOT_IN_USE;
    else {
        RID rid(GetPageNum(), slot);
        rec = RM_Record(GetRecordAddr(slot), rid, recordSize);
    }
    return 0;
}

RC RM_PageHandler::InsertRecord(char *pData, RID &rid) {
    //之前已经判断过当前页有空间可以插入了
    int slot = rm_pageHeader.firstFreeRec;
    rm_pageHeader.firstFreeRec = GetNextFreeSlot(slot);
    rm_pageHeader.NumOfRecords++;
    memcpy(pf_pageHandler.GetDataPtr(), &rm_pageHeader, sizeof(RM_PageHeader));
    assert(rm_pageHeader.NumOfRecords <= nMaxRecordPerPage);
    MarkSlotInUse(slot);
    memcpy(GetRecordAddr(slot), pData, recordSize);
    rid = RID(GetPageNum(), slot);
    return 0;
}

RC RM_PageHandler::UpdateRecord(char *pData, RID &rid) {
    int slot = rid.getSlot();
    assert(isRecordInUse(slot));    //断言slot被使用中
    memcpy(GetRecordAddr(slot), pData, recordSize);
    return 0;
}

RC RM_PageHandler::DeleteRecord(int slot) {

    char *pData = GetDataPtr();
    MarKSlotFree(slot);
    int next = GetNextFreeSlot(rm_pageHeader.firstFreeRec);
    *(int *)(&pData[slot *(recordSize + 1) + 1]) = next;
    rm_pageHeader.firstFreeRec = slot;  //修改free list
    rm_pageHeader.NumOfRecords--;   //记录数减一
    memcpy(pf_pageHandler.GetDataPtr(), &rm_pageHeader, sizeof(RM_PageHeader)); //RM模块文件头改变时记得写入内存!
    return 0;
}

int RM_PageHandler::GetNextFreeSlot(int slot) {
    char *pData = GetDataPtr();
    return *(int *)(&pData[slot *(recordSize + 1) + 1]);
}

void RM_PageHandler::InitPage() {
    rm_pageHeader.NumOfRecords = 0;
    rm_pageHeader.firstFreeRec = 0;
    memcpy(pf_pageHandler.GetDataPtr(), &rm_pageHeader, sizeof(RM_PageHeader));
    char *pData = GetDataPtr();
    int i;
    for(i = 0; i < nMaxRecordPerPage; i++) {
        pData[i * (recordSize + 1)] = 0;
        *(int *)(&pData[i * (recordSize + 1) + 1]) = i + 1;
    }
    *(int *)(&pData[(i - 1) * (recordSize + 1) + 1]) = RECORD_FREE_LIST_END;
}

RC RM_PageHandler::GetNextRecord(int cur_slot, RM_Record &rec) {
    int slot;
    int rc;
    for(slot = cur_slot + 1; slot < nMaxRecordPerPage; slot++) {
        if( (rc = GetRecord(slot, rec)) == RECORD_NOT_IN_USE ) {
            continue;
        }
        else return 0;
    }
    return RM_PAGE_RECORD_EOF;
}
