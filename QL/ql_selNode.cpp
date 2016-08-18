//
// Created by 谢俊东 on 16/7/30.
//

#include "ql.h"
#include "../RM/rm.h"
#include "CondFilter.h"

QL_SelNode::QL_SelNode(QL_Manager &qlm, QL_Node &prevNode, Condition cond) : QL_Node(qlm), prevNode(prevNode),
                                                                             cond(cond) {
    nAttrInfos = prevNode.GetAttrNum();
    attrInfos = new AttrInfoInRecord[nAttrInfos];

    memcpy(attrInfos, prevNode.GetAttrList(), sizeof(AttrInfoInRecord) * nAttrInfos);

}

void QL_SelNode::Open() {
    prevNode.Open();
}

RC QL_SelNode::GetNext(RM_Record &rec) {
    while (true) {
        if (prevNode.GetNext(rec) == QL_EOF) return QL_EOF;
        CondFilter condFilter(qlm, this->cond, 0);
        if(!condFilter.check(rec.GetContent())) continue;
        else return 0;
    }

}

void QL_SelNode::Close() {
    prevNode.Close();
}

void QL_SelNode::Reset() {
    prevNode.Reset();
}

QL_SelNode::~QL_SelNode() {
}