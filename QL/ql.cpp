//
// Created by 谢俊东 on 16/7/25.
//

#include <cassert>
#include "ql.h"

QL_Manager::QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm) : smm(smm), ixm(ixm), rmm(rmm) {
    ntotAttrInfo = 0;
}

RC QL_Manager::Select(int nSelAttrs, const RelAttr *selAttrs, int nRelations, const char *const *relations,
                      int nConditions, const Condition *conditions) {

    RC rc;
    //填充relcatTuples和AttrInfoInRecords数组,查询所需的必要信息(附带检查了表名是否存在)
    if ((rc = InitRelationAndAttrInfos(relations, nRelations))) return rc;

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
    //检查selAttrs中的属性是否都属于relations
    if ((rc = CheckAttrValid(nSelAttrs, selAttrs))) {
        return rc;
    }

    //检查conditions中的属性是否存在
    //属性和比较值(属性)的类型是否一致
    if ((rc = CheckCondsValid(nConditions, conditions))) return rc;

    int i, k;
    /*-------------------------------------------------------------*/
    //构建查询计划树
    vector<Condition> condsleft;    //不同表之间属性比较的condtion都放在这里面
    for (i = 0; i < nConditions; i++) condsleft.push_back(conditions[i]);

    vector< QL_RelNode > relNodes;
    for (i = 0; i < nRelations; i++) {
        relNodes.push_back(QL_RelNode(*this, relations[i]));
        for (auto it = begin(condsleft); it < end(condsleft); it++) {
            if ((*it).bRhsIsAttr) continue;

            AttrInfoInRecord attrInfoInRecord;
            GetAttrInfoByRelAttr(attrInfoInRecord, (*it).lhsAttr);
            if (!strcmp(attrInfoInRecord.relName, relations[i])) {
                if (attrInfoInRecord.indexNo != NO_INDEX) {
                    relNodes.back().AddIndexCondition(*it);
                } else {
                    relNodes.back().AddCondition(*it);
                }
                condsleft.erase(it);    //TODO it or it++?
            }

        }
    }
    vector<QL_JoinNode> joinNodes;

    QL_Node *topNode = &relNodes.front(); //topNode始终指向查询计划树的根节点
    for (i = 1; i < nRelations; i++) {
        joinNodes.push_back(QL_JoinNode(*this, *topNode, relNodes[i]));  //构建一颗左深树
        topNode = &joinNodes.back();
    }

    vector<QL_SelNode> selNodes;
    for (auto it = begin(condsleft); it < end(condsleft); it++) {
        selNodes.push_back(QL_SelNode(*this, *topNode, *it));
        topNode = &selNodes.at(i);
    }

    QL_ProjNode projNode(*this, *topNode, nSelAttrs, selAttrs);
    topNode = &projNode;        //query plan tree completed
    RM_Record rec;

    //fill DataAttrInfo数组
    DataAttrInfo *attributes = new DataAttrInfo[nSelAttrs];

    //填充attributes
    smm.FillDataAttrInfoForPrint(attributes, nSelAttrs, selAttrs, ntotAttrInfo, attrInfosArr);
    Printer p(attributes, nSelAttrs);
    p.PrintHeader(cout);
    topNode->Open();

    while (topNode->GetNext(rec) != QL_EOF) {
        p.Print(cout, rec.GetContent());
    }

    p.PrintFooter(cout);
    topNode->Close();
    CleanUp();
    return 0;
}

RC QL_Manager::Insert(const char *relName, int nValues, const Value *values) {
    RM_FileHandler rm_fileHandler;
    RM_FileScan fileScan;
    RC rc;
    if ((rc = InitRelationAndAttrInfos(&relName, 1)))
        return rc;

    rmm.OpenFile(relName, rm_fileHandler);

    //先检查是否会和primary属性冲突
    int i;
    RM_Record rec;
    RID rid;
    for (i = 0; i < ntotAttrInfo; i++) {
        if (attrInfosArr[i].isPrimary) {
            if (attrInfosArr[i].indexNo != NO_INDEX) {
                IX_IndexScan indexScan;
                IX_IndexHandler indexHandler;
                ixm.OpenIndex(relName, attrInfosArr[i].indexNo, indexHandler);
                indexScan.OpenScan(indexHandler, EQ_OP, values[i].data);
                if (indexScan.GetNextEntry(rid) != IX_EOF) {
                    indexScan.CloseScan();
                    return QL_PRIMARY_KEY_DUP;
                }
                indexScan.CloseScan();
                break;
            } else {
                fileScan.OpenScan(rm_fileHandler, attrInfosArr[i].attrType, attrInfosArr[i].attrLength,
                                  attrInfosArr[i].offset, EQ_OP, values[i].data);
                if (fileScan.GetNextRec(rec) != RM_EOF) {
                    fileScan.CloseScan();
                    return QL_PRIMARY_KEY_DUP;
                }
                fileScan.CloseScan();
                break;  //最多只有一个主键
            }
        }
    }

    char *buffer = (char *) calloc(relcatTuples->tupleLength, 1);
    int offset = 0;
    for (int i = 0; i < nValues; i++) {
        memcpy(buffer + offset, values[i].data, attrInfosArr[i].attrLength);
        offset += attrInfosArr[i].attrLength;
    }
    rm_fileHandler.InsertRec(buffer, rid);
    rmm.CloseFile(rm_fileHandler);
    //对index的处理
    bool bHasIndex = false;
    //判断有没有属性有index
    for (i = 0; i < ntotAttrInfo; i++) {
        if (attrInfosArr[i].indexNo != NO_INDEX) {
            bHasIndex = true;
            break;
        }
    }
    //如果有属性有索引,那么插入索引
    if (bHasIndex) {
        IX_IndexHandler ix_indexHandler;
        ixm.OpenIndex(relName, attrInfosArr[i].indexNo, ix_indexHandler);
        ix_indexHandler.InsertEntry(&buffer[attrInfosArr[i].offset], rid);
        ixm.CloseIndex(ix_indexHandler);
    }
    CleanUp();
    return 0;
}

RC QL_Manager::Update(const char *relName, const RelAttr &updAttr, const int bIsValue, const RelAttr &rhsRelAttr,
                      const Value &rhsValue, int nConditions, const Condition *conditions) {
    RC rc;
    /*----------------------语义检查-----------------------------------*/
    if ((rc = InitRelationAndAttrInfos(&relName, 1))) return rc;
    if ((rc = CheckAttrValid(1, &updAttr))) return rc;
    if (!bIsValue) { //如果右值是属性
        if ((rc = CheckAttrValid(1, &rhsRelAttr))) return rc;
    }
    if ((rc = CheckCondsValid(nConditions, conditions))) return rc;


    /*----------------------构建查询树-----------------------------------*/

    QL_Node *topNode;
    QL_RelNode *relNode = new QL_RelNode(*this, relName, Condition());
    topNode = relNode;
    int i, j;
    QL_SelNode *selNodes[nConditions];
    for (i = 0; i < nConditions; i++) {
        selNodes[i] = new QL_SelNode(*this, *topNode, conditions[i]);
        topNode = selNodes[i];
    }

    AttrInfoInRecord updAttrInfo;
    GetAttrInfoByRelAttr(updAttrInfo, updAttr);
    AttrInfoInRecord rhsAttrInfo;
    if (!bIsValue) {
        GetAttrInfoByRelAttr(rhsAttrInfo, rhsRelAttr);
        if (rhsAttrInfo.attrType != updAttrInfo.attrType) return QL_INCOMPATIBLE_COMP_OP;
    }

    //检查update的属性是否建立了索引,如果建立了索引,不能通过该属性indexscan来查询,在插入的同时要改变索引
    IX_IndexHandler ix_indexHandler;
    if (updAttrInfo.indexNo != NO_INDEX) {
        ixm.OpenIndex(relName, updAttrInfo.indexNo, ix_indexHandler);
    }
    //执行计划
    RM_Record rec;
    RM_FileHandler rm_fileHandler;
    rmm.OpenFile(relName, rm_fileHandler);

    DataAttrInfo *attributes = new DataAttrInfo[ntotAttrInfo];
    memcpy(attributes, attrInfosArr, sizeof(DataAttrInfo) * ntotAttrInfo);
    //填充attributes
    Printer p(attributes, ntotAttrInfo);
    p.PrintHeader(cout);
    topNode->Open();

    while (topNode->GetNext(rec) != QL_EOF) {
        char *buffer = rec.GetContent();
        if (bIsValue) {
            memcpy(&buffer[updAttrInfo.offset], rhsValue.data, updAttrInfo.attrLength);
        } else {
            memcpy(&buffer[updAttrInfo.offset], &buffer[rhsAttrInfo.offset],
                   min(updAttrInfo.attrLength, rhsAttrInfo.attrLength));
        }
        rm_fileHandler.UpdateRec(rec);
        p.Print(cout, buffer);

        //如果有索引,改变索引
        if (updAttrInfo.indexNo != NO_INDEX) {
            ix_indexHandler.DeleteEntry(&buffer[updAttrInfo.offset], rec.GetRid());
            ix_indexHandler.InsertEntry(bIsValue ? rhsValue.data : &buffer[rhsAttrInfo.offset], rec.GetRid());
        }
    }

    p.PrintFooter(cout);
    topNode->Close();
    rmm.CloseFile(rm_fileHandler);
    if (updAttrInfo.indexNo != NO_INDEX) ixm.CloseIndex(ix_indexHandler);
    CleanUp();
    return 0;
}

RC QL_Manager::InitRelationAndAttrInfos(const char *const *relName, int nRelations) {
    RC rc;
    relcatTuples = new RelcatTuple[nRelations];
    if ((rc = smm.FillRelCatTuples(relcatTuples, nRelations, relName)))
        return rc;
    this->nRelations = nRelations;
    int i;
    for (i = 0; i < nRelations; i++) {
        ntotAttrInfo += relcatTuples[i].attrCount;
        //查询所有表的属性数量之和
    }
    attrInfosArr = new AttrInfoInRecord[ntotAttrInfo];

    smm.FillAttrInfoInRecords(attrInfosArr, nRelations, relcatTuples);
    return 0;
}

RC QL_Manager::Delete(const char *relName, int nConditions, const Condition *conditions) {

    RC rc;
    if ((rc = InitRelationAndAttrInfos(&relName, 1))) return rc;
    if ((rc = CheckCondsValid(nConditions, conditions))) return rc;


    /*----------------------构建查询树-----------------------------------*/
    QL_Node *topNode;
    QL_RelNode *relNode = new QL_RelNode(*this, relName, Condition());
    topNode = relNode;
    int i, j;
    QL_SelNode *selNodes[nConditions];
    for (i = 0; i < nConditions; i++) {
        selNodes[i] = new QL_SelNode(*this, *topNode, conditions[i]);
        topNode = selNodes[i];
    }

    //执行计划
    IX_IndexHandler ix_indexHandler;
    bool bHasIndex = false;
    int indexedAttrNo;
    for (i = 0; i < ntotAttrInfo; i++) {
        if (attrInfosArr[i].indexNo != NO_INDEX) {
            bHasIndex = true;
            indexedAttrNo = i;
            break;
        }
    }
    if (bHasIndex) ixm.OpenIndex(relName, attrInfosArr[indexedAttrNo].indexNo, ix_indexHandler);
    RM_Record rec;
    RM_FileHandler rm_fileHandler;
    rmm.OpenFile(relName, rm_fileHandler);

    DataAttrInfo *attributes = new DataAttrInfo[ntotAttrInfo];
    memcpy(attributes, attrInfosArr, sizeof(DataAttrInfo) * ntotAttrInfo);
    //填充attributes
    Printer p(attributes, ntotAttrInfo);
    p.PrintHeader(cout);
    topNode->Open();

    while (topNode->GetNext(rec) != QL_EOF) {
        char *buffer = rec.GetContent();
        rm_fileHandler.DeleteRec(rec.GetRid());
        p.Print(cout, buffer);
        if (bHasIndex) {
            ix_indexHandler.DeleteEntry(&buffer[attrInfosArr[indexedAttrNo].offset], rec.GetRid());
        }
    }
    p.PrintFooter(cout);
    topNode->Close();
    rmm.CloseFile(rm_fileHandler);
    if (bHasIndex) ixm.CloseIndex(ix_indexHandler);
    CleanUp();
    return 0;
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

RC QL_Manager::CheckAttrValid(int nAttrs, const RelAttr *Attrs) {
    int i, k;
    for (i = 0; i < nAttrs; i++) {
        if (Attrs[i].relName == NULL) {  //如果没有指定表名
            for (k = 0; k < ntotAttrInfo; k++) {
                if (!strcmp(Attrs[i].attrName, attrInfosArr[k].attrName))
                    break;
            }
            if (k >= ntotAttrInfo) return QL_ATTR_NOT_EXIST;

        } else {    //如果指定了表名
            for (k = 0; k < ntotAttrInfo; k++) {
                if (!strcmp(Attrs[i].relName, attrInfosArr[k].relName) &&
                    !strcmp(Attrs[i].attrName, attrInfosArr[k].attrName))
                    break;
            }
            if (k >= ntotAttrInfo) return QL_ATTR_NOT_EXIST;
        }
    }
    return 0;
}


RC QL_Manager::CheckCondsValid(int nConditions, const Condition *conditions) {
    RC rc;
    RelAttr *Attrs = new RelAttr[nConditions * 2];
    int nAttrs = 0;
    int i;
    for (i = 0; i < nConditions; i++) {
        Attrs[nAttrs++] = conditions[i].lhsAttr;
        if (conditions[i].bRhsIsAttr) {  //如果右值是属性
            Attrs[nAttrs++] = conditions[i].rhsAttr;
        }
    }
    if ((rc = CheckAttrValid(nAttrs, Attrs))) {
        return rc;
    }
    for (i = 0; i < nConditions; i++) {
        if (!conditions[i].bRhsIsAttr) {
            //如果右值是常量
            if (GetAttrType(conditions[i].lhsAttr) != conditions[i].rhsValue.type) return QL_INCOMPATIBLE_COMP_OP;
        } else {
            //如果右值是属性
            if (GetAttrType(conditions[i].lhsAttr) != GetAttrType(conditions[i].rhsAttr))
                return QL_INCOMPATIBLE_COMP_OP;
        }
    }
    delete[] Attrs;
    return 0;
}


AttrType QL_Manager::GetAttrType(const RelAttr &relAttr) {
    assert(attrInfosArr != NULL);
    AttrInfoInRecord attrInfo;
    GetAttrInfoByRelAttr(attrInfo, relAttr);
    return attrInfo.attrType;
}

void QL_Manager::CleanUp() {
    if (relcatTuples) {
        delete[] relcatTuples;
    }
    if (attrInfosArr) {
        delete[] attrInfosArr;
    }
}


void QL_Manager::GetAttrInfoByRelAttr(AttrInfoInRecord &attrInfo, const RelAttr &relAttr) {
    int i;
    for (i = 0; i < ntotAttrInfo; i++) {
        if (relAttr.relName) {
            if (!strcmp(relAttr.relName, attrInfosArr[i].relName) &&
                !strcmp(relAttr.attrName, attrInfosArr[i].attrName)) {
                attrInfo = attrInfosArr[i];
                break;
            }
        } else {
            if (!strcmp(relAttr.attrName, attrInfosArr[i].attrName)) {
                attrInfo = attrInfosArr[i];
                break;
            }

        }
    }

}

const char *ql_error_msg[] = {"duplicate attribute name", "duplicate table name", "attribute not exist",
                              "table not exist",
                              "incompatible compare operators", "priamry key duplicate", "QL EOF"};

void QL_PrintError(RC rc) {
    printf("Error: %s\n", ql_error_msg[START_QL_ERR - 1 - rc]);
}

