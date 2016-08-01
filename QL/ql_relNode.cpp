//
// Created by 谢俊东 on 16/7/30.
//


#include "ql.h"

QL_RelNode::QL_RelNode(QL_Manager &qlm, const char *const relation) : QL_Node(qlm) {
    relName = new char[MAXNAME + 1];
    strcpy(relName, relation);
    int i;
    relCatTuple = new RelcatTuple[1];
    for(i = 0; i < qlm.nRelations; i++) {
        if(!strcmp(relName, qlm.relcatTuples[i].relName)) {
            memcpy(relCatTuple, &qlm.relcatTuples[i], sizeof(RelcatTuple));
            break;
        }
    }

    tupleLength = relCatTuple->tupleLength;
    nAttrInfos = relCatTuple->attrCount;
    attrInfos = new AttrInfoInRecord[nAttrInfos];

    qlm.smm.FillAttrInfoInRecords(attrInfos, 1, relCatTuple);

}

void QL_RelNode::Open() {
    qlm.rmm.OpenFile(relName, rm_fileHandler);
    rm_fileScan.OpenScan(rm_fileHandler, INT, 4, 0, NO_OP, NULL);
}

RC QL_RelNode::GetNext(RM_Record &rec) {
    if (rm_fileScan.GetNextRec(rec) == RM_EOF) return QL_EOF;
    return 0;
}

void QL_RelNode::Close() {
    rm_fileScan.CloseScan();
    qlm.rmm.CloseFile(rm_fileHandler);
}

void QL_RelNode::Reset() {
    rm_fileScan.CloseScan();
    rm_fileScan.OpenScan(rm_fileHandler, INT, 4, 0, NO_OP, NULL);
}

QL_RelNode::~QL_RelNode() {
    if (relName) delete[] relName;
    delete[] relCatTuple;
    delete[] attrInfos;
}

