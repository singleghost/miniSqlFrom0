//
// Created by 谢俊东 on 16/7/27.
//

#ifndef MINISQLFROM0_QL_NODE_H
#define MINISQLFROM0_QL_NODE_H

#include "ql_manager.h"
#include "../RM/rm.h"


//抽象类, 查询计划树的节点
class QL_Node { //抽象node类
public:
    virtual ~QL_Node() {}
    //作为迭代器的三个方法, 纯虚函数,继承类必须实现!!!
    virtual void Open() = 0;
    virtual RC GetNext(RM_Record &rec) = 0;
    virtual void Close() = 0;
    virtual void Reset() = 0;

protected:
    int tupleLength;
    AttrInfoInRecord attrInfoTuples[];
    int nAttrInfoTuples;
//    int *attrsIndexInRec;   //记录这个节点中所有属性的下标,通过下标到QL_Manager中的AttrInfoInRecord数组中定位实际的属性
//    int attrsCount;         //这个节点的属性数量
//    Condition cond;
};

//投影node
class QL_ProjNode : public QL_Node {
    //TODO正确实现或禁用赋值运算符和复制构造函数
public:
    QL_ProjNode(QL_Manager &qlm, QL_Node &prevNode, int nAttrs, const RelAttr projAttrs[]) :
            qlm(qlm), prevNode(prevNode) , nAttrs(nAttrs)
    {
        this->projAttrs = new AttrInfoInRecord[nAttrs];
        memcpy(this->projAttrs, projAttrs, nAttrs * sizeof(RelAttr));
    }
    virtual ~QL_ProjNode() {delete [] projAttrs; }
    virtual void Open();
    virtual RC GetNext(RM_Record &rec);
    virtual void Close();

private:

    QL_Manager &qlm;
    QL_Node &prevNode;      //前一个节点
};

//选择节点, Condition中的条件
class QL_SelNode : public QL_Node {
public:
    QL_SelNode(QL_Manager &qlm, QL_Node &prevNode, Condition cond);
    virtual ~QL_SelNode() {}
    virtual void Open();
    virtual RC GetNext(RM_Record &rec);
    virtual void Close();

private:
    QL_Manager &qlm;
    QL_Node &prevNode;  //前一个节点
    Condition cond;   //选择条件
    RM_FileScan rm_fileScan;

    AttrInfoInRecord leftAttr;
    AttrInfoInRecord rightAttr;
};

//Join节点,有左右两个子节点,右节点是relNode, 左节点可以是relNode或joinNode
class QL_JoinNode : public QL_Node {
public:
    QL_JoinNode(QL_Manager &qlm, QL_Node &leftNode, QL_Node &rightNode) :qlm(qlm), lSubNode(leftNode), rSubNode(rightNode) {}
    virtual ~QL_JoinNode() {}
    virtual void Open();
    virtual RC GetNext(RM_Record &rec);
    virtual void Close();

private:
    QL_Manager &qlm;
    QL_Node &lSubNode;          //左节点
    QL_Node &rSubNode;       //右节点

    bool bRightNodeEOF;
    char *buffer;
//    bool bLeftSubNodeIsJoin;    //指示左节点是joinNode还是relNode
//    Condition nConds;           //带条件的Join
};

class QL_RelNode : public QL_Node {
public:
    QL_RelNode(QL_Manager &qlm, const char *const relation);
    virtual ~QL_RelNode();
    virtual void Open();
    virtual RC GetNext(RM_Record &rec);
    virtual void Close();
    virtual void Reset();

private:
    QL_Manager &qlm;
    char *relName;              //关系名
    RelcatTuple *relCatTuple;
    RM_FileHandler rm_fileHandler;
    RM_FileScan rm_fileScan;

};

#endif //MINISQLFROM0_QL_NODE_H
