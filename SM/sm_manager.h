//
// Created by 谢俊东 on 16/7/22.
//

#ifndef MINISQLFROM0_SM_MANAGER_H
#define MINISQLFROM0_SM_MANAGER_H

#include "../minisql.h"
#include "../IX/ix.h"
#include "printer.h"
#include "../QL/ql_manager.h"

#define RELCAT_RECORD_SIZE (MAXNAME + sizeof(int) * 3)
#define ATTRCAT_RECORD_SIZE (MAXNAME + MAXNAME + sizeof(int) + sizeof(AttrType) + sizeof(int) + sizeof(int))
#define NO_INDEX -1 //attrcat表中属性上没有建索引时indexNo的值

//RC code
#define SM_TABLE_NOT_EXIST -1
#define SM_DUPLICATE_TABLE -2
#define SM_DUPLICATE_INDEX -3
#define SM_DATA_FILE_NOT_EXIST -4
#define SM_ATTR_NOT_FOUND -5
#define SM_NO_INDEX_ON_ATTR -6
// Used by SM_Manager::CreateTable
struct AttrInfo {
    char     *attrName;           // Attribute name
    AttrType attrType;            // Type of attribute
    int      attrLength;          // Length of attribute
};


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
};

class SM_Manager {
private:
    IX_Manager &ixm;
    RM_Manager &rmm;

    RM_FileHandler relcat_fhandler;
    RM_FileHandler attrcat_fhandler;

    RM_FileScan rm_fileScan;
    static void createRelCatTuple(const char *relName, int tupleLength, int attrCount, int indexCount, char *relcat_rec);
    static void createAttrCatTuple(const char *relName, const char *attrName, int offset, AttrType attrType, int attrLength, int indexNo, char *attrcat_rec);

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

    //一些提供给QL的接口
    bool IsAttrInOneOfRelations(const char *attrName, int nRelations, const char * const relations[]);
    bool IsRelationExist(const char *relName);
    void FillDataAttrInfoForPrint(DataAttrInfo *attrInfos, int nAttrs, const RelAttr *relAttrs, int nRelations, const char * const relations[]);
    AttrType GetAttrType(const char *relName, const char *attrName);
    void FillRelCatTuples(RelcatTuple *relcatTuples, int nRelations, const char * const relations[]);
    void FillAttrInfoInRecords(AttrInfoInRecord *attrInfoInRecords, int nRelations, const RelcatTuple *relCatTuples);
};

void SM_PrintError(RC rc);


#endif //MINISQLFROM0_SM_MANAGER_H
