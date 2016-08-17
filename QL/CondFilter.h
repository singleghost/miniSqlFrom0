//
// Created by 谢俊东 on 16/8/16.
//

#ifndef MINISQLFROM0_CONDFILTER_H
#define MINISQLFROM0_CONDFILTER_H


#include "../Parse/parser.h"
class QL_Manager;
class CondFilter {
public:
    CondFilter(QL_Manager &qlm, const Condition &cond);
    bool check(char *rec);

private:
    QL_Manager &qlm;
    Condition cond;
    AttrInfoInRecord leftAttr;
    AttrInfoInRecord rightAttr;
    bool (*compfunc)(const void *value1, const void *value2, AttrType attrType, int attrLength);    //比较函数
};


#endif //MINISQLFROM0_CONDFILTER_H
