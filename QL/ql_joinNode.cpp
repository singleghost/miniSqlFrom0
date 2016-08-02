//
// Created by 谢俊东 on 16/7/30.
//

#include "ql.h"

QL_JoinNode::QL_JoinNode(QL_Manager &qlm, QL_Node &leftNode, QL_Node &rightNode)
        : QL_Node(qlm), lSubNode(leftNode), rSubNode(rightNode)
{
    AttrInfoInRecord *leftAttrInfos;
    AttrInfoInRecord *rightAttrInfos;
    lSubNode.GetAttrList(leftAttrInfos);
    rSubNode.GetAttrList(rightAttrInfos);

    tupleLength = lSubNode.GetTupleLength() + rSubNode.GetTupleLength();
    nAttrInfos = lSubNode.GetAttrNum() + rSubNode.GetAttrNum();
    attrInfos = new AttrInfoInRecord[nAttrInfos];

    memcpy(attrInfos, leftAttrInfos, sizeof(AttrInfoInRecord) * lSubNode.GetAttrNum());
    memcpy(attrInfos + lSubNode.GetAttrNum(), rightAttrInfos,
           sizeof(AttrInfoInRecord) * rSubNode.GetAttrNum());
    int i;
    for(i = 0; i < rSubNode.GetAttrNum(); i++) {
        attrInfos[lSubNode.GetAttrNum() + i].offset += lSubNode.GetTupleLength();
    }
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
        if (lSubNode.GetNext(leftRec) == QL_EOF)
            return QL_EOF;
        memcpy(buffer, leftRec.GetContent(), leftRec.GetRecordSize());
        bRightNodeEOF = false;
    }
    if (rSubNode.GetNext(rightRec) == QL_EOF) {
        bRightNodeEOF = true;
        rSubNode.Reset();
        goto nextTurn;
    }
    memcpy(buffer + lSubNode.GetTupleLength(), rightRec.GetContent(), rSubNode.GetTupleLength());
    RID rid(-1, -1);
    rec = RM_Record(buffer, rid, tupleLength);
    return 0;
}

void QL_JoinNode::Close() {
    lSubNode.Close();
    rSubNode.Close();
}

void QL_JoinNode::Reset() {
    lSubNode.Reset();
    rSubNode.Reset();
}