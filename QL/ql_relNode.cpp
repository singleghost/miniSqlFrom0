//
// Created by 谢俊东 on 16/7/30.
//


#include "ql.h"
#include "CondFilter.h"

QL_RelNode::QL_RelNode(QL_Manager &qlm, const char *const relation) : QL_Node(qlm), relName(relation) {
    InitRelNodeInfos();
    useIndexScan = false;

}

QL_RelNode::QL_RelNode(QL_Manager &qlm, const char *const relation, const Condition &indexCond)
        : QL_Node(qlm), relName(relation), indexCond(indexCond) {
    InitRelNodeInfos();
    useIndexScan = true;
}

void QL_RelNode::InitRelNodeInfos() {
    int i;
    relCatTuple = shared_ptr<RelcatTuple>(new RelcatTuple[1]);
    for (i = 0; i < qlm.nRelations; i++) {
        if (relName == qlm.relcatTuples[i].relName) {
            memcpy(relCatTuple.get(), &qlm.relcatTuples[i], sizeof(RelcatTuple));
            break;
        }
    }
    tupleLength = relCatTuple->tupleLength;
    nAttrInfos = relCatTuple->attrCount;
    attrInfos = new AttrInfoInRecord[nAttrInfos];

    qlm.smm.FillAttrInfoInRecords(attrInfos, 1, relCatTuple.get());
}

void QL_RelNode::Open() {
    qlm.rmm.OpenFile(relName, rm_fileHandler);
    if (useIndexScan) {
        qlm.ixm.OpenIndex(relName, leftAttr.indexNo, ix_indexHandler);
        ix_indexScan.OpenScan(ix_indexHandler, indexCond.op, indexCond.rhsValue.data);
    } else {
        rm_fileScan.OpenScan(rm_fileHandler, INT, 4, 0, NO_OP, NULL);
    }
}

RC QL_RelNode::GetNext(RM_Record &rec) {
    RID rid;
    RC rc;
    while (true) {
        if (useIndexScan) {
            if ((rc = ix_indexScan.GetNextEntry(rid)))
                return QL_EOF;
            if ((rc = rm_fileHandler.GetRec(rid, rec))) {
                printf("index scan, get record error\n");
                return rc;

            }
        } else {
            if (rm_fileScan.GetNextRec(rec) == RM_EOF)
                return QL_EOF;
        }
        for (Condition cond : otherConds) {
            CondFilter condFilter(qlm, cond);
            if (!condFilter.check(rec.GetContent())) continue;
        }
        return 0;
    }
}

void QL_RelNode::Close() {
    if (useIndexScan) {
        ix_indexScan.CloseScan();
        qlm.ixm.CloseIndex(ix_indexHandler);
    } else {
        rm_fileScan.CloseScan();
    }
    qlm.rmm.CloseFile(rm_fileHandler);
}

void QL_RelNode::Reset() {
    if (useIndexScan) {
        ix_indexScan.CloseScan();
        ix_indexScan.OpenScan(ix_indexHandler, indexCond.op, indexCond.rhsValue.data);
    } else {
        rm_fileScan.CloseScan();
        rm_fileScan.OpenScan(rm_fileHandler, INT, 4, 0, NO_OP, NULL);
    }
}

QL_RelNode::~QL_RelNode() {
}

void QL_RelNode::AddCondition(const Condition &cond) {
    otherConds.push_back(cond);
}

void QL_RelNode::AddIndexCondition(const Condition &cond) {
    qlm.GetAttrInfoByRelAttr(leftAttr, cond.lhsAttr);
    useIndexScan = true;
    indexCond = cond;
}