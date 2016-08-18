//
// Created by 谢俊东 on 16/8/16.
//

#include "CondFilter.h"
#include "../SM/printer.h"
#include "ql.h"
#include "../minisql.h"
#include "../RM/rm.h"

CondFilter::CondFilter(QL_Manager &qlm, const Condition &cond, int relOffset) : qlm(qlm), cond(cond){

    //确定用于比较的函数
    switch (cond.op) {
        case EQ_OP: compfunc = &myComp::equal_To; break;
        case LT_OP: compfunc = &myComp::less_than; break;
        case LE_OP: compfunc = &myComp::less_than_or_equal; break;
        case GT_OP: compfunc = &myComp::greater_than; break;
        case GE_OP: compfunc = &myComp::greater_than_or_equal; break;
        case NE_OP: compfunc = &myComp::not_equal_to; break;
        case NO_OP: printf("SelNode don't support no op!\n"); break;
    }
    //初始化leftAttr和rightAttr
    //TODO bug!!! attrInfo 里面的offset是不对的!
    this->qlm.GetAttrInfoByRelAttr(leftAttr, cond.lhsAttr);
    leftAttr.offset -= relOffset;
    if (cond.bRhsIsAttr) {  //如果右值是属性
        this->qlm.GetAttrInfoByRelAttr(rightAttr, cond.rhsAttr);
        rightAttr.offset -= relOffset;
    }
}

bool CondFilter::check(char *buffer) {

    if (cond.bRhsIsAttr) {   //右边是属性
        if (leftAttr.attrType == STRING) {
            int maxerLen = max(leftAttr.attrLength, rightAttr.attrLength);
            char rhsBuffer[maxerLen + 1];
            memset(rhsBuffer, 0, sizeof(rhsBuffer));
            memcpy(rhsBuffer, &buffer[rightAttr.offset], rightAttr.attrLength);
            char lhsBuffer[maxerLen + 1];
            memset(lhsBuffer, 0, sizeof(lhsBuffer));
            memcpy(lhsBuffer, &buffer[leftAttr.offset], leftAttr.attrLength);
            assert(leftAttr.attrType == rightAttr.attrType);
            return compfunc(lhsBuffer, rhsBuffer, leftAttr.attrType, maxerLen);

        } else {
            return compfunc(&buffer[leftAttr.offset], &buffer[rightAttr.offset], leftAttr.attrType,
                            max(leftAttr.attrLength, rightAttr.attrLength));
        }
    } else {
        //右边是值
        //如果是字符串比较,要先进行0字节填充
        if (leftAttr.attrType == STRING) {
            if (strlen(cond.rhsValue.data) > leftAttr.attrLength) return false;
            char rhsBuffer[leftAttr.attrLength + 1];
            memset(rhsBuffer, 0, sizeof(rhsBuffer));
            memcpy(rhsBuffer, cond.rhsValue.data, strlen(cond.rhsValue.data));
            char lhsBuffer[leftAttr.attrLength + 1];
            memset(lhsBuffer, 0, sizeof(lhsBuffer));
            memcpy(lhsBuffer, &buffer[leftAttr.offset], leftAttr.attrLength);
            return compfunc(lhsBuffer, rhsBuffer, leftAttr.attrType, leftAttr.attrLength);
        } else {
            return compfunc(&buffer[leftAttr.offset], cond.rhsValue.data, leftAttr.attrType, leftAttr.attrLength);
        }
    }
}

