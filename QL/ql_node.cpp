//
// Created by 谢俊东 on 16/7/28.
//

#include "ql.h"

/*-------------------------------抽象类QL_Node-------------------------------------*/
void QL_Node::GetAttrList(AttrInfoInRecord *&attrList) {
    attrList = attrInfos;
}
