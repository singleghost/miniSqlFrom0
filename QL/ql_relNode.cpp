//
// Created by 谢俊东 on 16/7/30.
//


#include "ql.h"

QL_RelNode::QL_RelNode(QL_Manager &qlm, const char *const relation, const Condition &cond,
                       bool hasCond) : QL_Node(qlm), hasCond(hasCond), cond(cond)
{
    relName = new char[MAXNAME + 1];
    strcpy(relName, relation);
    int i;
    relCatTuple = new RelcatTuple[1];
    for (i = 0; i < qlm.nRelations; i++) {
        if (!strcmp(relName, qlm.relcatTuples[i].relName)) {
            memcpy(relCatTuple, &qlm.relcatTuples[i], sizeof(RelcatTuple));
            break;
        }
    }

    tupleLength = relCatTuple->tupleLength;
    nAttrInfos = relCatTuple->attrCount;
    attrInfos = new AttrInfoInRecord[nAttrInfos];

    qlm.smm.FillAttrInfoInRecords(attrInfos, 1, relCatTuple);
    if (hasCond) {
        int i;
        //初始化leftAttr
        for (i = 0; i < nAttrInfos; i++) {
            if (cond.lhsAttr.relName != NULL) {
                if (!strcmp(cond.lhsAttr.attrName, attrInfos[i].attrName) &&
                    !strcmp(cond.lhsAttr.relName, attrInfos[i].relName)) {
                    leftAttr = attrInfos[i];
                    break;
                }

            } else {
                if (!strcmp(cond.lhsAttr.attrName, attrInfos[i].attrName)) {
                    leftAttr = attrInfos[i];
                    break;
                }
            }

        }
        assert(cond.bRhsIsAttr == false);   //右值不能是属性
    }
    if (leftAttr.isPrimary) useIndex = true;
    else useIndex = false;
}

void QL_RelNode::Open() {
    qlm.rmm.OpenFile(relName, rm_fileHandler);
    if (useIndex) {
        qlm.ixm.OpenIndex(relName, leftAttr.indexNo, ix_indexHandler);
        ix_indexScan.OpenScan(ix_indexHandler, cond.op, cond.rhsValue.data);
    } else {
        if (!hasCond) {
            rm_fileScan.OpenScan(rm_fileHandler, INT, 4, 0, NO_OP, NULL);
        } else {
            rm_fileScan.OpenScan(rm_fileHandler, leftAttr.attrType, leftAttr.attrLength, leftAttr.offset, cond.op,
                                 cond.rhsValue.data);
        }
    }
}

RC QL_RelNode::GetNext(RM_Record &rec) {
    RID rid;
    RC rc;
    if (useIndex) {
        if (ix_indexScan.GetNextEntry(rid) != 0)
            return QL_EOF;
        if( rc = rm_fileHandler.GetRec(rid, rec)) {
            printf("index scan, get record error\n");

        }
    } else {
        if (rm_fileScan.GetNextRec(rec) == RM_EOF)
            return QL_EOF;
    }
    return 0;
}

void QL_RelNode::Close() {
    if (useIndex) {
        ix_indexScan.CloseScan();
        qlm.ixm.CloseIndex(ix_indexHandler);
    } else {
        rm_fileScan.CloseScan();
    }
    qlm.rmm.CloseFile(rm_fileHandler);
}

void QL_RelNode::Reset() {
    if (useIndex) {
        ix_indexScan.CloseScan();
        ix_indexScan.OpenScan(ix_indexHandler, cond.op, cond.rhsValue.data);
    } else {
        rm_fileScan.CloseScan();
        if (!hasCond) {
            rm_fileScan.OpenScan(rm_fileHandler, INT, 4, 0, NO_OP, NULL);
        }
        else {
            rm_fileScan.OpenScan(rm_fileHandler, leftAttr.attrType, leftAttr.attrLength, leftAttr.offset, cond.op,
                                 cond.rhsValue.data);
        }
    }
}

QL_RelNode::~QL_RelNode() {
    if (relName) delete[] relName;
    delete[] relCatTuple;
    delete[] attrInfos;
}

