//
// Created by 谢俊东 on 16/7/25.
//

#include "ql_manager.h"
#include "ql_node.h"
#include "../SM/printer.h"

QL_Manager::QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm) : smm(smm), ixm(ixm), rmm(rmm){
    nAttrInfoInRecord = 0;
}

RC QL_Manager::Select(int nSelAttrs, const RelAttr *selAttrs, int nRelations, const char *const *relations,
                      int nConditions, const Condition *conditions) {
    //语义检查
    /*-------------------------------------------------------------*/
    //检查selattrs中有无重复的属性
    if (HasDupAttrName(nSelAttrs, selAttrs)) {
        return QL_DUP_ATTR_NAME;
    }
    //检查Relations中有无重复的表名
    if (HasDupTableName(nRelations, relations)) {
        return QL_DUP_TABLE_NAME;
    }
    //检查relations中的表名是否都存在
    if (!CheckTablesValid(nRelations, relations)) {
        return QL_TABLE_NOT_EXIST;
    }

    //检查selAttrs中的属性是否都属于relations
    if (!CheckAttrValid(nSelAttrs, selAttrs, nRelations, relations)) {
        return QL_ATTR_NOT_EXIST;
    }

    //检查conditions中的属性是否存在
    if (!CheckCondAttrValid(nRelations, relations, nConditions, conditions)) {
        return QL_ATTR_NOT_EXIST;
    }

    //属性和比较值(属性)的类型是否一致
    if (!CheckCondCompTypeConsistent(nConditions, conditions)) {
        return QL_INCOMPATIBLE_COMP_OP;
    }

    /*-------------------------------------------------------------*/
    //构建查询计划树
    int i, j;
    QL_RelNode *relNodes[nRelations];
    for (i = 0; i < nRelations; i++) {
        relNodes[i] = new QL_RelNode(*this, relations[i]);
    }
    QL_JoinNode *joinNodes[nRelations - 1];

    QL_Node *topNode = relNodes[0]; //topNode始终指向查询计划树的根节点
    for (i = 1; i < nRelations; i++) {
        joinNodes[i - 1] = JoinTwoNode(*topNode, *relNodes[i]);  //构建一颗左深树
    }

    QL_SelNode *selNodes[nConditions];
    for (i = 0; i < nConditions; i++) {
        selNodes[i] = new QL_SelNode(*this, *topNode, conditions[i]);
        topNode = selNodes[i];
    }

    QL_ProjNode projNode(*this, *topNode, nSelAttrs, selAttrs);
    topNode = &projNode;        //query plan tree completed
    RM_Record rec;

    //填充relcatTuples和AttrInfoInRecords数组,查询所需的必要信息
    relcatTuples = new RelcatTuple[nRelations];
    smm.FillRelCatTuples(relcatTuples, nRelations, relations);

    for(i = 0; i < nRelations; i++) {
        nAttrInfoInRecord += relcatTuples[i].attrCount;
        //查询所有表的属性数量之和
    }
    attrInfoTuples = new AttrInfoInRecord[nAttrInfoInRecord];
    smm.FillAttrInfoInRecords(attrInfoTuples, nRelations, relcatTuples);

    //fill DataAttrInfo数组
    //...

    DataAttrInfo *attributes = new DataAttrInfo[nSelAttrs];
    //填充attributes
    smm.FillDataAttrInfoForPrint(attributes, nSelAttrs, selAttrs, nRelations, relations);
    Printer p(attributes, nSelAttrs);
    p.PrintHeader(cout);
    topNode->Open();

    while(topNode->GetNext(rec) != QL_EOF) {
        p.Print(cout, rec.GetContent());

    }

    p.PrintFooter(cout);
    topNode->Close();

}

QL_JoinNode * QL_Manager::JoinTwoNode(QL_Node &lRelNode, QL_Node &rRelNode) {
    QL_JoinNode *joinNode = new QL_JoinNode(*this, lRelNode, rRelNode);
    return joinNode;
}

bool QL_Manager::HasDupAttrName(int nAttrs, const RelAttr *Attrs) {
    int i, j;
    for (i = 0; i < nAttrs; i++) {
        for (j = i + 1; j < nAttrs; j++) {
            if (!(Attrs[i].relName != NULL && Attrs[j].relName != NULL)) {
                if (strcmp(Attrs[i].attrName, Attrs[j].attrName) == 0) {
                    return true;
                }
            } else if (Attrs[i].relName != NULL && Attrs[j].relName != NULL) {
                if (strcmp(Attrs[i].relName, Attrs[j].relName) == 0 &&
                    strcmp(Attrs[i].attrName, Attrs[j].attrName) == 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool QL_Manager::HasDupTableName(int nRelations, const char *const *relations) {
    int i, j;
    for (i = 0; i < nRelations; i++) {
        for (j = i + 1; j < nRelations; j++) {
            if (strcmp(relations[i], relations[j]) == 0) {
                return true;
            }
        }
    }
    return false;
}

bool QL_Manager::CheckAttrValid(int nAttrs, const RelAttr *Attrs, int nRelations, const char *const relations[]) {
    int i, j;
    for (i = 0; i < nAttrs; i++) {
        if (Attrs[i].relName != NULL) {  //如果没有指定表名
            if (!smm.IsAttrInOneOfRelations(Attrs[i].attrName, nRelations, relations)) {
                return false;
            }
        } else {    //如果指定了表名
            if (!smm.IsAttrInOneOfRelations(Attrs[i].attrName, 1, (const char *const *) &Attrs[i].relName)) {
                return false;
            }
        }
    }
    return true;
}

bool QL_Manager::CheckTablesValid(int nRelations, const char *const *relations) {
    int i;
    for (i = 0; i < nRelations; i++) {
        if (!smm.IsRelationExist(relations[i])) return false;
    }
    return true;
}

bool QL_Manager::CheckCondAttrValid(int nRelations, const char *const *relations, int nCondions,
                                    const Condition *conditions) {
    RelAttr *Attrs = new RelAttr[nCondions * 2];
    int nAttrs = 0;
    int i;
    for (i = 0; i < nCondions; i++) {
        Attrs[nAttrs++] = conditions[i].lhsAttr;
        if (conditions[i].bRhsIsAttr) {  //如果右值是属性
            Attrs[nAttrs++] = conditions[i].rhsAttr;
        }
    }
    if (!CheckAttrValid(nAttrs, Attrs, nRelations, relations)) {
        return false;
    }

}

bool QL_Manager::CheckCondCompTypeConsistent(int nConditions, const Condition *conditions) {
    int i;
    for (i = 0; i < nConditions; i++) {
        if (!conditions[i].bRhsIsAttr) {
            //如果右值是常量
            if (smm.GetAttrType(conditions[i].lhsAttr.relName, conditions[i].lhsAttr.attrName) !=
                conditions[i].rhsValue.type) {
                return false;
            }
        }
    }

}