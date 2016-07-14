//
// Created by 谢俊东 on 16/7/14.
//

#include "rm.h"

RM_PageHandler::RM_PageHandler(PageHandler &ph, RM_FileHeader rm_fileHeader){
    pf_pageHandler = ph;
    recordSize = rm_fileHeader.RecordSize;
    nMaxRecordPerPage = rm_fileHeader.nMaxRecordsPerPage;
    pData = ph.GetDataPtr() + sizeof(rm_pageHeader);
}

RC RM_PageHandler::GetRecord(int slot, RM_Record &rec) {
    char * pData = this->pf_pageHandler.GetDataPtr();
    if (pData[slot *(recordSize + 1)] == 0) return RECORD_NOT_IN_USE;
    else {
        RID rid(pf_pageHandler.GetPageNum(), slot);
        rec = RM_Record(&pData[slot * (recordSize + 1) + 1], rid, recordSize);
    }
    return 0;
}

RC RM_PageHandler::InsertRecord(char *pData, RID &rid) {
    //之前已经判断过当前页有空间可以插入了
    int slot = rm_pageHeader.firstFreeRec;
    rm_pageHeader.firstFreeRec = GetNextFreeSlot(slot);
    MarkSlotInUse(slot);
    memcpy(GetRecordAddr(slot), pData, recordSize);
    rid = RID(GetPageNum(), slot);
    return 0;
}

RC RM_PageHandler::UpdataRecord(char *pData, RID &rid) {
    int slot = rid.getSlot();
    memcpy(GetRecordAddr(slot), pData, recordSize);
    return 0;
}

int RM_PageHandler::GetNextFreeSlot(int slot) {
    char *pData = GetDataPtr();
    return *(int *)(&pData[slot *(recordSize + 1) + 1]);
}

void RM_PageHandler::InitPage() {
    rm_pageHeader.NumOfRecords = 0;
    rm_pageHeader.firstFreeRec = 0;
    char *pData = GetDataPtr();
    int i;
    for(i = 0; i < nMaxRecordPerPage; i++) {
        pData[i * (recordSize + 1)] = 0;
        *(int *)(&pData[i * (recordSize + 1) + 1]) = i + 1;
    }
    *(int *)(&pData[(i - 1) * (recordSize + 1) + 1]) = RECORD_FREE_LIST_END;
}

RC RM_PageHandler::DeleteRecord(int slot) {

    char *pData = GetDataPtr();
    pData[slot *(recordSize + 1)] = SLOT_FREE;
    rm_pageHeader.NumOfRecords--;   //记录数减一
    int next = GetNextFreeSlot(rm_pageHeader.firstFreeRec);
    *(int *)(&pData[slot *(recordSize + 1) + 1]) = next;
    rm_pageHeader.firstFreeRec = slot;  //修改free list

    return 0;
}