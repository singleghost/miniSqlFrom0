//
// Created by 谢俊东 on 16/7/25.
//

#include "ql.h"

QL_Manager::QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm) : smm(smm), ixm(ixm), rmm(rmm){
    nAttrInfoInRecord = 0;
}

RC QL_Manager::Select(int nSelAttrs, const RelAttr *selAttrs, int nRelations, char **relations,
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
    attrInfoArray = new AttrInfoInRecord[nAttrInfoInRecord];
    smm.FillAttrInfoInRecords(attrInfoArray, nRelations, relcatTuples);

    //fill DataAttrInfo数组

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
    CleanUp();
    return 0;
}

RC QL_Manager::Insert(const char *relName, int nValues, const Value *values) {
    RM_FileHandler rm_fileHandler;
    rmm.OpenFile(relName, rm_fileHandler);

    relcatTuples = new RelcatTuple;
    smm.FillRelCatTuples(relcatTuples, 1, &relName);
    nAttrInfoInRecord = relcatTuples->attrCount;
    attrInfoArray = new AttrInfoInRecord[relcatTuples->attrCount];
    smm.FillAttrInfoInRecords(attrInfoArray, 1, relcatTuples);

    char *buffer = new char[relcatTuples->tupleLength];
    int offset = 0;
    for(int i = 0; i < nValues; i++) {
        memcpy(buffer + offset, values[i].data, attrInfoArray[i].attrLength);
        offset += attrInfoArray[i].attrLength;
    }
    RID rid;
    rm_fileHandler.InsertRec(buffer, rid);
    return 0;
}

RC QL_Manager::Update(const char *relName, const RelAttr &updAttr, const int bIsValue, const RelAttr &rhsRelAttr,
                      const Value &rhsValue, int nConditions, const Condition *conditions) {
    /*----------------------语义检查-----------------------------------*/
    if(!CheckTablesValid(1, &relName)) return QL_TABLE_NOT_EXIST;
    if(!CheckAttrValid(1, &updAttr, 1, &relName)) return QL_ATTR_NOT_EXIST;
    if(!bIsValue) { //如果右值是属性
        if (!CheckAttrValid(1, &rhsRelAttr, 1, &relName)) return QL_ATTR_NOT_EXIST;
    }
    if(!CheckCondAttrValid(1, &relName, nConditions, conditions)) return QL_ATTR_NOT_EXIST;
    if(!CheckCondCompTypeConsistent(nConditions, conditions)) return QL_INCOMPATIBLE_COMP_OP;

    /*----------------------构建查询树-----------------------------------*/
    QL_Node *topNode;
    QL_RelNode *relNode = new QL_RelNode(*this, relName);
    topNode = relNode;
    int i,j;
    QL_SelNode *selNodes[nConditions];
    for(i = 0; i < nConditions; i++) {
        selNodes[i] = new QL_SelNode(*this, *topNode, conditions[i]);
        topNode = selNodes[i];
    }
    relcatTuples = new RelcatTuple;
    smm.FillRelCatTuples(relcatTuples, 1, &relName);
    nAttrInfoInRecord = relcatTuples->attrCount;
    attrInfoArray = new AttrInfoInRecord[relcatTuples->attrCount];

    smm.FillAttrInfoInRecords(attrInfoArray, 1, relcatTuples);
    AttrInfoInRecord updAttrInfo;
    GetAttrInfoByRelAttr(updAttrInfo, updAttr);
    AttrInfoInRecord rhsAttrInfo;
    if(!bIsValue) {
        GetAttrInfoByRelAttr(rhsAttrInfo, rhsRelAttr);
        if(rhsAttrInfo.attrType != updAttrInfo.attrType) return QL_INCOMPATIBLE_COMP_OP;
    }
    //执行计划
    RM_Record rec;
    RM_FileHandler rm_fileHandler;
    rmm.OpenFile(relName, rm_fileHandler);

    DataAttrInfo *attributes = new DataAttrInfo[nAttrInfoInRecord];
    memcpy(attributes, attrInfoArray, sizeof(DataAttrInfo) * nAttrInfoInRecord);
    //填充attributes
    Printer p(attributes, nAttrInfoInRecord);
    p.PrintHeader(cout);
    topNode->Open();

    while(topNode->GetNext(rec) != QL_EOF) {
        char *buffer = rec.GetContent();
        if (bIsValue) {
            memcpy(&buffer[updAttrInfo.offset], rhsValue.data, updAttrInfo.attrLength);
        } else {
            memcpy(&buffer[updAttrInfo.offset], &buffer[rhsAttrInfo.offset],
                   min(updAttrInfo.attrLength, rhsAttrInfo.attrLength));
        }
        rm_fileHandler.UpdateRec(rec);
        p.Print(cout, buffer);
    }
    p.PrintFooter(cout);
    topNode->Close();
    CleanUp();
    return 0;
}

RC QL_Manager::Delete(const char *relName, int nConditions, const Condition *conditions) {
    if(!CheckTablesValid(1, &relName)) return QL_TABLE_NOT_EXIST;
    if(!CheckCondAttrValid(1, &relName, nConditions, conditions)) return QL_ATTR_NOT_EXIST;
    if(!CheckCondCompTypeConsistent(nConditions, conditions)) return QL_INCOMPATIBLE_COMP_OP;


    /*----------------------构建查询树-----------------------------------*/
    QL_Node *topNode;
    QL_RelNode *relNode = new QL_RelNode(*this, relName);
    topNode = relNode;
    int i,j;
    QL_SelNode *selNodes[nConditions];
    for(i = 0; i < nConditions; i++) {
        selNodes[i] = new QL_SelNode(*this, *topNode, conditions[i]);
        topNode = selNodes[i];
    }
    relcatTuples = new RelcatTuple;
    smm.FillRelCatTuples(relcatTuples, 1, &relName);
    nAttrInfoInRecord = relcatTuples->attrCount;
    attrInfoArray = new AttrInfoInRecord[relcatTuples->attrCount];
    smm.FillAttrInfoInRecords(attrInfoArray, 1, relcatTuples);

    //执行计划
    RM_Record rec;
    RM_FileHandler rm_fileHandler;
    rmm.OpenFile(relName, rm_fileHandler);

    DataAttrInfo *attributes = new DataAttrInfo[nAttrInfoInRecord];
    memcpy(attributes, attrInfoArray, sizeof(DataAttrInfo) * nAttrInfoInRecord);
    //填充attributes
    Printer p(attributes, nAttrInfoInRecord);
    p.PrintHeader(cout);
    topNode->Open();

    while(topNode->GetNext(rec) != QL_EOF) {
        char *buffer = rec.GetContent();
        rm_fileHandler.DeleteRec(rec.GetRid());
        p.Print(cout, buffer);
    }
    p.PrintFooter(cout);
    topNode->Close();
    CleanUp();
    return 0;
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
    return true;
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
    return true;
}

void QL_Manager::CleanUp(){
    if(relcatTuples) {
        delete [] relcatTuples;
    }
    if(attrInfoArray) {
        delete [] attrInfoArray;
    }
}


void QL_Manager::GetAttrInfoByRelAttr(AttrInfoInRecord &attrInfo, const RelAttr &relAttr) {
    int i;
    for(i = 0; i < nAttrInfoInRecord; i++) {
        if(relAttr.relName) {
            if(!strcmp(relAttr.relName, attrInfoArray[i].relName) &&
               !strcmp(relAttr.attrName, attrInfoArray[i].attrName)) {
                attrInfo = attrInfoArray[i];
                break;
            }
        } else {
            if(!strcmp(relAttr.attrName, attrInfoArray[i].attrName)) {
                attrInfo = attrInfoArray[i];
                break;
            }

        }
    }

}
