//
// Created by 谢俊东 on 16/7/30.
//

#include "ql.h"
#include "../RM/rm.h"

QL_SelNode::QL_SelNode(QL_Manager &qlm, QL_Node &prevNode, Condition cond) : QL_Node(qlm), prevNode(prevNode), cond(cond) {
    //确定用于比较的函数
    switch (cond.op) {
        case EQ_OP:
            compfunc = &myComp::equal_To;
            break;
        case LT_OP:
            compfunc = &myComp::less_than;
            break;
        case LE_OP:
            compfunc = &myComp::less_than_or_equal;
            break;
        case GT_OP:
            compfunc = &myComp::greater_than;
            break;
        case GE_OP:
            compfunc = &myComp::greater_than_or_equal;
            break;
        case NE_OP:
            compfunc = &myComp::not_equal_to;
            break;
        case NO_OP:
            printf("SelNode don't support no op!\n");
            break;
    }
    nAttrInfos = prevNode.GetAttrNum();
    attrInfos = new AttrInfoInRecord[nAttrInfos];

    AttrInfoInRecord *prevAttrInfos;
    prevNode.GetAttrList(prevAttrInfos);
    memcpy(attrInfos, prevAttrInfos, sizeof(AttrInfoInRecord) * nAttrInfos);

    int i;
    //初始化leftAttr和rightAttr
    for (i = 0; i < nAttrInfos; i++) {
        if (cond.lhsAttr.relName != NULL) {
            if (!strcmp(cond.lhsAttr.attrName, prevAttrInfos[i].attrName) &&
                !strcmp(cond.lhsAttr.relName, prevAttrInfos[i].relName)) {
                leftAttr = prevAttrInfos[i];
                break;
            }

        } else {
            if (!strcmp(cond.lhsAttr.attrName, prevAttrInfos[i].attrName)) {
                leftAttr = prevAttrInfos[i];
                break;
            }
        }

    }
    if (cond.bRhsIsAttr) {  //如果右值是属性
        for (i = 0; i < nAttrInfos; i++) {
            if (cond.rhsAttr.relName != NULL) {
                if (!strcmp(cond.rhsAttr.attrName, prevAttrInfos[i].attrName) &&
                    !strcmp(cond.rhsAttr.relName, prevAttrInfos[i].relName)) {
                    rightAttr = prevAttrInfos[i];
                    break;
                }

            } else {
                if (!strcmp(cond.rhsAttr.attrName, qlm.attrInfosArr[i].attrName)) {
                    rightAttr = prevAttrInfos[i];
                    break;
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
            //TODO
            if (compfunc(&buffer[leftAttr.offset], &buffer[rightAttr.offset], leftAttr.attrType,
                               max(leftAttr.attrLength, rightAttr.attrLength))) {
                return 0;
            }
        } else {
            //右边是值
            //如果是字符串比较,要先进行0字节填充
            //TODO bug exist
            if(leftAttr.attrType == STRING) {
                if(strlen((char *)cond.rhsValue.data) > leftAttr.attrLength) continue;
                char rhsBuffer[leftAttr.attrLength + 1];
                memset(rhsBuffer, 0, leftAttr.attrLength);
                memcpy(rhsBuffer, cond.rhsValue.data, strlen(cond.rhsValue.data));
                char lhsBuffer[leftAttr.attrLength + 1];
                memset(lhsBuffer, 0, leftAttr.attrLength);
                memcpy(lhsBuffer, &buffer[leftAttr.offset], leftAttr.attrLength);
                if (compfunc(lhsBuffer, rhsBuffer, leftAttr.attrType, leftAttr.attrLength)) {
                    return 0;
                }
            } else if (compfunc(&buffer[leftAttr.offset], cond.rhsValue.data, leftAttr.attrType, leftAttr.attrLength)) {
                return 0;
            }
        }
    }

}

void QL_SelNode::Close() {
    prevNode.Close();
}

void QL_SelNode::Reset() {
    prevNode.Reset();
}

QL_SelNode::~QL_SelNode() {
    delete [] attrInfos;
}