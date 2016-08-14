//
// Created by 谢俊东 on 16/7/22.
//

#ifndef MINISQLFROM0_SM_MANAGER_H
#define MINISQLFROM0_SM_MANAGER_H

#include "../minisql.h"
#include "../IX/ix.h"
#include "printer.h"
#include "../Parse/parser.h"

//RC code
#define SM_TABLE_NOT_EXIST (START_SM_ERR-1)
#define SM_DUPLICATE_TABLE (START_SM_ERR-2)
#define SM_DUPLICATE_INDEX (START_SM_ERR-3)
#define SM_DATA_FILE_NOT_EXIST (START_SM_ERR-4)
#define SM_ATTR_NOT_FOUND (START_SM_ERR-5)
#define SM_NO_INDEX_ON_ATTR (START_SM_ERR-6)
#define SM_PRIMARY_KEY_DUP (START_SM_ERR-7)

struct RelcatTuple {
    char relName[MAXNAME];
    int tupleLength;
    int attrCount;
    int indexCount;
};

struct AttrcatTuple {
    char relName[MAXNAME];
    char attrName[MAXNAME];
    int offset;
    AttrType attrType;
    int attrLength;
    int indexNo;
    int bIsPrimary;
};

#define RELCAT_RECORD_SIZE (sizeof(RelcatTuple))
#define ATTRCAT_RECORD_SIZE (sizeof(AttrcatTuple))
#define NO_INDEX -1 //attrcat表中属性上没有建索引时indexNo的值

class SM_Manager {
    friend class QL_Manager;
    friend class QL_RelNode;
    friend class SyntaxAnalyser;
private:
    IX_Manager &ixm;
    RM_Manager &rmm;

    RM_FileHandler relcat_fhandler;
    RM_FileHandler attrcat_fhandler;

    RM_FileScan rm_fileScan;

    //转换函数,把从attrcat表中得到的原始元祖转换为AttrInfoInRecord结构体或DataAttrInfo结构体(两个结构体除了名字和字段含义其他完全一样)
    void ConvertFromAttrCatToAttrInfo(AttrInfoInRecord *attrInfo, AttrcatTuple *attrcatTuple);
    void ConvertFromAttrCatToAttrInfo(DataAttrInfo *attrInfo, AttrcatTuple *attrcatTuple);

    //一些提供给QL的接口
    bool IsAttrInOneOfRelations(const char *attrName, int nRelations, const char *const *relations, int &relNum);
    bool IsRelationExist(const char *relName);
    void FillDataAttrInfoForPrint(DataAttrInfo *dataAttrInfos, int nAttrs, const RelAttr *relAttrs, int nAttrInfos,
                                  AttrInfoInRecord *attrInfos);
    AttrType GetAttrType(const char *relName, const char *attrName, int nrelations, const char *const *relations);
    void FillRelCatTuples(RelcatTuple *relcatTuples, int nRelations, const char * const relations[]);
    void FillAttrInfoInRecords(AttrInfoInRecord *attrInfos, int nRelations, const RelcatTuple *relCatTuples);
    void FillRelAttrs(int nRelations, const char *const *relations, vector<RelAttr> &relAttrs);

public:
    SM_Manager  (IX_Manager &ixm, RM_Manager &rmm) : ixm(ixm), rmm(rmm) {}  // Constructor
    ~SM_Manager () {};                                  // Destructor
    RC OpenDb      (const char *dbName);                // Open database
    RC CloseDb     ();                                  // Close database
    RC CreateTable (const char *relName,                // Create relation
                    int        attrCount,
                    AttrInfo   *attributes);
    RC DropTable   (const char *relName);               // Destroy relation
    RC CreateIndex (const char *relName,                // Create index
                    const char *attrName);
    RC DropIndex   (const char *relName,                // Destroy index
                    const char *attrName);
    RC Load        (const char *relName,                // Load utility
                    const char *fileName);
    RC Help        ();                                  // Help for database
    RC Help        (const char *relName);               // Help for relation
    RC Print       (const char *relName);               // Print relation
    RC Set         (const char *paramName,              // Set system parameter
                    const char *value);


    static void createRelCatTuple(const char *relName, int tupleLength, int attrCount, int indexCount, char *relcat_rec);

    static void createAttrCatTuple(const char *relName, const char *attrName, int offset, AttrType attrType, int attrLength,
                                       int indexNo, int bIsPrimary, char *attrcat_rec);
};

void SM_PrintError(RC rc);


#endif //MINISQLFROM0_SM_MANAGER_H
