//
// Created by 谢俊东 on 16/7/30.
//

#include <cassert>
#include "ql.h"

QL_ProjNode::QL_ProjNode(QL_Manager &qlm, QL_Node &prevNode, int nAttrs, const RelAttr projAttrs[]) :
        QL_Node(qlm), prevNode(prevNode)
{
    AttrInfoInRecord *prevAttrList;
    prevNode.GetAttrList(prevAttrList);
    int nPrevAttrs = prevNode.GetAttrNum();

    int i, j;
    tupleLength = 0;

//    if(nAttrs == 1) {
//        if(projAttrs[0].relName == nullptr && !strcmp(projAttrs[0].attrName, "*")) {
//            nAttrInfos = qlm.ntotAttrInfo;
//            this->attrInfos = new AttrInfoInRecord[nAttrInfos];
//            memcpy(this->attrInfos, qlm.attrInfosArr, sizeof(AttrInfoInRecord) * qlm.ntotAttrInfo);
//            this->offsetInPrev = new int[nAttrs];
//            for(j = 0; j < nPrevAttrs; j++) {
//                offsetInPrev[j] = prevAttrList[j].offset;
//                tupleLength += prevAttrList->attrLength;
//            }
//            buffer = new char[tupleLength];
//            return ;
//        }
//
//    }
    nAttrInfos = nAttrs;
    this->attrInfos = new AttrInfoInRecord[nAttrs];
    this->offsetInPrev = new int[nAttrs];
    for (i = 0; i < nAttrs; i++) {
        for (j = 0; j < nPrevAttrs; j++) {
            if (projAttrs[i].relName) {
                if (!strcmp(projAttrs[i].attrName, prevAttrList[j].attrName) &&
                    !strcmp(projAttrs[i].relName, prevAttrList[j].relName)) {
                    memcpy(&attrInfos[i], &prevAttrList[j], sizeof(AttrInfoInRecord));
                    attrInfos[i].offset = tupleLength;
                    tupleLength += attrInfos[i].attrLength;
                    offsetInPrev[i] = prevAttrList[j].offset;
                    break;
                }

            } else {
                if(!strcmp(projAttrs[i].attrName, prevAttrList[j].attrName)) {
                    memcpy(&attrInfos[i], &prevAttrList[j], sizeof(AttrInfoInRecord));
                    attrInfos[i].offset = tupleLength;
                    tupleLength += attrInfos[i].attrLength;
                    offsetInPrev[i] = prevAttrList[j].offset;
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
    AttrInfoInRecord *prevAttrInfos;
    prevNode.GetAttrList(prevAttrInfos);
    RM_Record record;
    if (prevNode.GetNext(record) == QL_EOF) {
        return QL_EOF;
    }
    int i;
    int base = 0;
    char *prevRecord = record.GetContent();
    //投影操作
    for(i = 0; i < nAttrInfos; i++) {
        memcpy(buffer + base, prevRecord + offsetInPrev[i], attrInfos[i].attrLength);
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
