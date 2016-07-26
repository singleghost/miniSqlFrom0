#include "../PF/pf_filemgr.h"
#include "sm_manager.h"

//
// Created by 谢俊东 on 16/7/23.
//
#define DBNAME "ZJU"
int main()
{
    printf("%d", strcmp(NULL, NULL));
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    IX_Manager ixm(pfm);
    SM_Manager smm(ixm, rmm);

    smm.OpenDb(DBNAME);
    AttrInfo attrs[3];
    attrs[0].attrType = STRING;
    attrs[0].attrLength = 30;
    attrs[0].attrName = "departmentName";

    attrs[1].attrType = INT;
    attrs[1].attrLength = 4;
    attrs[1].attrName = "buildYear";

    attrs[2].attrType = FLOAT;
    attrs[2].attrLength = 4;
    attrs[2].attrName = "budget";

    RC rc;
    if(rc = smm.DropTable("departments")) {
        SM_PrintError(rc);
        exit(1);
    }
    if(rc = smm.CreateTable("departments", 3, attrs)) {
        SM_PrintError(rc);
    }

    if(rc = smm.CreateTable("departments", 3, attrs)) {
        SM_PrintError(rc);
    }
//    if(rc = smm.CreateIndex("departments", "departmentName")) printf("rc=%d\n", rc);
//    if(rc = smm.CreateIndex("departments", "buildYear")) printf("rc=%d\n", rc);
//    if(rc = smm.CreateIndex("departments", "budget")) printf("rc=%d\n", rc);
    if(rc = smm.DropIndex("departments", "departmentName")) {
        SM_PrintError(rc);
    }
//    smm.DropTable("departments");
//    smm.Load("departments", "/Users/dddong/workspace/ClionProjects/miniSqlFrom0/dep.data");
    smm.Print("departments");
    smm.Print("relcat");
    smm.Print("attrcat");
    if(rc = smm.CloseDb()) {
        SM_PrintError(rc);
        exit(1);
    }
}

