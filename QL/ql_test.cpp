//
// Created by 谢俊东 on 16/7/29.
//

#include "ql.h"

int main() {
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    IX_Manager ixm(pfm);
    SM_Manager smm(ixm, rmm);
    QL_Manager qlm(smm, ixm, rmm);

    printf("begin test\n\n\n");
    smm.OpenDb("ZJU");

//    smm.Print("departments");
//    smm.Print("relcat");
//    smm.Print("attrcat");
    RC rc;
    int Year = 1990;
    float budget = 20000.5;
    Value values[3] = {{STRING, (void *) "Computer Science"},
                       {INT,    &Year},
                       {FLOAT,  &budget}};
    char *relations[1] = {"departments"};

    AttrInfo attrs[3] = {
            {"departmentName", STRING, 30},
            {"buildYear",      INT,    4},
            {"budget",         FLOAT,  4}
    };

    if (rc = smm.DropTable("departments")) {
        SM_PrintError(rc);
//        exit(1);
    }
    if (rc = smm.CreateTable("departments", 3, attrs)) {
        SM_PrintError(rc);
    }
    if ((rc = qlm.Insert("departments", 3, values))) {
        PrintError(rc);
    }

    Year = 1999;
    budget = 44000;
    Value values2[3] = {{STRING, (void *) "Mathematics"},
                        {INT,    &Year},
                        {FLOAT,  &budget}};

    if ((rc = qlm.Insert("departments", 3, values2))) {
        PrintError(rc);
    }

    Year = 1980;
    budget = 50000.55;
    Value values3[3] = {{STRING, (void *) "Software Engineering"},
                        {INT,    &Year},
                        {FLOAT,  &budget}};

    if ((rc = qlm.Insert("departments", 3, values3))) {
        PrintError(rc);
    }

    RelAttr *selAttrs = new RelAttr[2];
//    selAttrs[0].relName = NULL;
    selAttrs[0].relName = NULL;
    selAttrs[1].relName = NULL;
//    selAttrs[0].attrName = new char[MAXNAME];
    selAttrs[0].attrName = new char[MAXNAME];
    selAttrs[1].attrName = new char[MAXNAME + 1];
//    memcpy(selAttrs[0].attrName, "departmentName", strlen("departmentName"));
    memcpy(selAttrs[0].attrName, "buildYear", strlen("buildYear"));
    memcpy(selAttrs[1].attrName, "departmentName", strlen("departmentName") + 1);

    int cmpYear = 1990;
    Value cmpValue = {STRING, (void *) "Computer Science"};
    Value cmpValue2 = {INT,
                       &cmpYear};
    Condition conds[2] = {{
                                  {NULL,          "departmentName"},
                                  GT_OP,
                                  0,
                                  {NULL, NULL},
                                  cmpValue,
                          },
                          {
                                  {"departments", "buildYear"},
                                  NE_OP,
                                  0,
                                  {NULL, NULL},
                                  cmpValue2,
                          }};
    smm.Print("departments");
    printf("\n\n");

    if ((rc = qlm.Select(2, selAttrs, 1, (const char *const *) relations, 2,
                         conds))) {
        PrintError(rc);
    }
    int upYear = 2000;
    RelAttr upAttr = { NULL, "buildYear"};
    RelAttr rhsAttr = {NULL, NULL};
    Value rhsValue = { INT, &upYear};
//    if(rc = qlm.Update("departments", upAttr, 1, rhsAttr, rhsValue, 0, NULL)) PrintError(rc);
    if(rc = qlm.Delete("departments", 2, conds));
    printf("\n\n");
    smm.Print("departments");
    smm.CloseDb();
}