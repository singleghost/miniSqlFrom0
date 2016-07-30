//
// Created by 谢俊东 on 16/7/30.
//

#include "ql.h"

QL_ProjNode::QL_ProjNode(QL_Manager &qlm, QL_Node &prevNode, int nAttrs, const RelAttr projAttrs[]) :
        QL_Node(qlm), prevNode(prevNode)
{
    AttrInfoInRecord *prevAttrList;
    prevNode.GetAttrList(prevAttrList);
    int nPrevAttrs = prevNode.GetAttrNum();

    nAttrInfos = nAttrs;
    tupleLength = 0;
    this->attrInfos = new AttrInfoInRecord[nAttrs];
    int i, j;
    for (i = 0; i < nAttrs; i++) {
        for (j = 0; j < nPrevAttrs; j++) {
            if (projAttrs[i].relName) {
                if (!strcmp(projAttrs[i].attrName, prevAttrList[j].attrName) &&
                    !strcmp(projAttrs[i].relName, prevAttrList[j].relName)) {
                    memcpy(&attrInfos[i], &prevAttrList[j], sizeof(AttrInfoInRecord));
                    attrInfos[i].offset = tupleLength;
                    tupleLength += attrInfos[i].attrLength;
                    break;
                }

            } else {
                if(!strcmp(projAttrs[i].attrName, prevAttrList[j].attrName)) {
                    memcpy(&attrInfos[i], &prevAttrList[j], sizeof(AttrInfoInRecord));
                    attrInfos[i].offset = tupleLength;
                    tupleLength += attrInfos[i].attrLength;
                    break;
                }
            }
        }
    }
    buffer = new char[tupleLength];
}

QL_ProjNode::~QL_ProjNode() {
    delete [] buffer;
    delete [] attrInfos;
}

void QL_ProjNode::Open() {
    prevNode.Open();

}

RC QL_ProjNode::GetNext(RM_Record &rec) {
    RM_Record record;
    if (prevNode.GetNext(record) == QL_EOF) {
        return QL_EOF;
    }
    int i;
    int base = 0;
    char *prevRecord = rec.GetContent();
    for(i = 0; i < nAttrInfos; i++) {
        memcpy(buffer + base, prevRecord + attrInfos[i].offset, attrInfos[i].attrLength);
        base += attrInfos[i].attrLength;
    }
    RID rid(-1, -1);
    rec = RM_Record(buffer, rid, tupleLength);
    return 0;
}

void QL_ProjNode::Close() {
    prevNode.Close();
}

void QL_ProjNode::Reset() {
    prevNode.Reset();
}
