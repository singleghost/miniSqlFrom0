//
// Created by 谢俊东 on 16/7/28.
//

#include "ql_node.h"
#include "../RM/rm.h"

QL_RelNode::QL_RelNode(QL_Manager &qlm, const char *const relation) : qlm(qlm) {
    relName = new char[strlen(relation + 1)];
    strcpy(relName, relation);
    relCatTuple = new RelcatTuple[1];
    qlm.smm.FillRelCatTuples(relCatTuple, 1, &relation);
    tupleLength = relCatTuple->tupleLength;
    nAttrInfoTuples = relCatTuple->attrCount;
    attrInfoTuples = new AttrInfoInRecord[nAttrInfoTuples];

    qlm.smm.FillAttrInfoInRecords(attrInfoTuples, 1, relCatTuple);

}

void QL_RelNode::Open() {
    qlm.rmm.OpenFile(relName, rm_fileHandler);
    rm_fileScan.OpenScan(rm_fileHandler, INT, 4, 0, NO_OP, NULL);
}

RC QL_RelNode::GetNext(RM_Record &rec) {
    if (rm_fileScan.GetNextRec(rec) == RM_EOF) return QL_EOF;
}

void QL_RelNode::Close() {
    rm_fileScan.CloseScan();
    qlm.rmm.CloseFile(rm_fileHandler);
}

void QL_RelNode::Reset() {
    rm_fileScan.CloseScan();
    rm_fileScan.OpenScan(rm_fileHandler, INT, 4, 0, NO_OP, NULL);
}

virtual QL_RelNode::~QL_RelNode() {
    if (relName) delete[] relName;
    delete[] relCatTuple;
    delete[] attrInfoTuples;
}

QL_SelNode::QL_SelNode(QL_Manager &qlm, QL_Node &prevNode, Condition cond) : qlm(qlm), prevNode(prevNode), cond(cond) {
    switch (cond.op) {
        case EQ_OP:
            cond.compartor = &equal;
            break;
        case LT_OP:
            cond.compartor = &less_than;
            break;
        case LE_OP:
            cond.compartor = &less_than_or_equal;
            break;
        case GT_OP:
            cond.compartor = &greater_than;
            break;
        case GE_OP:
            cond.compartor = &greater_than_or_equal;
            break;
        case NE_OP:
            cond.compartor = &not_equal;
            break;
    }
    int i, j;
    for (i = 0; i < qlm.nAttrInfoInRecord; i++) {
        if (cond.lhsAttr.relName != NULL) {
            if (!strcmp(cond.lhsAttr.attrName, qlm.attrInfoTuples[i].attrName) &&
                !strcmp(cond.lhsAttr.relName, qlm.attrInfoTuples[i].relName)) {
                leftAttr = qlm.attrInfoTuples[i];

            }

        } else {
            if (!strcmp(cond.lhsAttr.attrName, qlm.attrInfoTuples[i].attrName)) {
                leftAttr = qlm.attrInfoTuples[i];
            }
        }

    }
    if (cond.bRhsIsAttr) {
        for (i = 0; i < qlm.nAttrInfoInRecord; i++) {
            if (cond.rhsAttr.relName != NULL) {
                if (!strcmp(cond.rhsAttr.attrName, qlm.attrInfoTuples[i].attrName) &&
                    !strcmp(cond.rhsAttr.relName, qlm.attrInfoTuples[i].relName)) {
                    rightAttr = qlm.attrInfoTuples[i];

                }

            } else {
                if (!strcmp(cond.rhsAttr.attrName, qlm.attrInfoTuples[i].attrName)) {
                    rightAttr = qlm.attrInfoTuples[i];
                }
            }

        }
    }
}

void QL_SelNode::Open() {
    prevNode.Open();
}

RC QL_SelNode::GetNext(RM_Record &rec) {
    while (true) {
        if (prevNode.GetNext(rec) == QL_EOF) return QL_EOF;
        char *buffer = rec.GetContent();
        if (cond.bRhsIsAttr) {   //右边是属性
            if (cond.compartor(&buffer[leftAttr.offset], &buffer[rightAttr.offset], leftAttr.attrType,
                               leftAttr.attrLength)) {
                return 0;
            }
        } else {
            //右边是值
            if (cond.compartor(&buffer[leftAttr.offset], &cond.rhsValue, leftAttr.attrType, leftAttr.attrLength)) {
                return 0;
            }
        }
    }

}

void QL_SelNode::Close() {
    prevNode.Close();
}

QL_JoinNode::QL_JoinNode(QL_Manager &qlm, QL_Node &leftNode, QL_Node &rightNode) : qlm(qlm), lSubNode(leftNode),
                                                                                   rSubNode(rightNode) {
    tupleLength = lSubNode.tupleLength + rSubNode.tupleLength;
    attrInfoTuples = new AttrInfoInRecord[lSubNode.nAttrInfoTuples + rSubNode.nAttrInfoTuples];
    memcpy(attrInfoTuples, lSubNode.attrInfoTuples, sizeof(AttrInfoInRecord) * lSubNode.nAttrInfoTuples);
    memcpy(attrInfoTuples + sizeof(AttrInfoInRecord) * lSubNode.nAttrInfoTuples, rSubNode.attrInfoTuples,
           sizeof(AttrInfoInRecord) * rSubNode.nAttrInfoTuples);
    buffer = new char[tupleLength];
}

void QL_JoinNode::Open() {
    lSubNode.Open();
    rSubNode.Open();
    bRightNodeEOF = true;
}

RC QL_JoinNode::GetNext(RM_Record &rec) {
    RM_Record leftRec;
    RM_Record rightRec;

    nextTurn:
    if (bRightNodeEOF) {
        if(lSubNode.GetNext(leftRec) == QL_EOF)
            return QL_EOF;
        memcpy(buffer, leftRec.GetContent(), leftRec.GetRecordSize());
        bRightNodeEOF = false;
    }
    if (rSubNode.GetNext(rightRec) == QL_EOF) {
        bRightNodeEOF = true;
        rSubNode.Reset();
        goto nextTurn;
    }
    memcpy(buffer + lSubNode.tupleLength, rightRec.GetContent(), rSubNode.tupleLength);
    RID rid(-1, -1);
    rec = RM_Record(buffer, rid, tupleLength);
    return 0;
}

void QL_JoinNode::Close() {
    lSubNode.Close();
    rSubNode.Close();
}

QL_ProjNode::QL_ProjNode(QL_Manager &qlm, QL_Node &prevNode, int nAttrs, const RelAttr projAttrs[]) :
qlm(qlm), prevNode(prevNode) , nAttrInfoTuples(nAttrs)
{
    this->attrInfoTuples = new AttrInfoInRecord[nAttrs];
    //TODO
}

void QL_ProjNode::Open() {
    prevNode.Open();

}

RC QL_ProjNode::GetNext(RM_Record &rec) {
    RM_Record record;
    if(prevNode.GetNext(record) == QL_EOF) {
        return QL_EOF;
    }
}