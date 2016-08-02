//
// Created by 谢俊东 on 16/7/29.
//

#include "ql.h"

#define NSELATTRS 8
#define NSELRELS 3
#define NCONDS 3

int main() {
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    IX_Manager ixm(pfm);
    SM_Manager smm(ixm, rmm);
    QL_Manager qlm(smm, ixm, rmm);

    printf("begin test\n\n\n");
    smm.OpenDb("ZJU");

    RC rc;
    int Year = 1990;
    float budget = 20000.5;
    Value values[3] = {{STRING, (void *) "Computer Science"},
                       {INT,    &Year},
                       {FLOAT,  &budget}};
    char *relations[NSELRELS] = {"departments", "students", "takes"};

    AttrInfo attrs[3] = {
            {"departmentName", STRING, 30},
            {"buildYear",      INT,    4},
            {"budget",         FLOAT,  4}
    };

    AttrInfo attrs2[3] = {
            {"id",   STRING, 20},
            {"name", STRING, 40},
            {"age",  INT,    4},
    };

    AttrInfo attrs3[2] = {
            {"id",     STRING, 25},
            {"course", STRING, 50}
    };
    if (rc = smm.DropTable("takes")) {
        SM_PrintError(rc);
    }
    if (rc = smm.CreateTable("takes", 2, attrs3)) SM_PrintError(rc);
    Value value_take1[2] = {{STRING, (void *) "3140104773"},
                            {STRING, (void *) "计算机组成"}};
    if (rc = qlm.Insert("takes", 2, value_take1)) SM_PrintError(rc);

    Value value_take2[2] = {{STRING, (void *) "3140104775"},
                            {STRING, (void *) "现代管理基础"}};
    if (rc = qlm.Insert("takes", 2, value_take2)) SM_PrintError(rc);

    if (rc = smm.DropTable("students")) {
        SM_PrintError(rc);
//        exit(1);
    }
    if (rc = smm.CreateTable("students", 3, attrs2)) {
        SM_PrintError(rc);
    }

    int age = 21;
    Value values_stu[3] = {{STRING, (void *) "3140104775"},
                           {STRING, (void *) "KaiWeng Chu"},
                           {INT,    &age}};

    if ((rc = qlm.Insert("students", 3, values_stu))) {
        PrintError(rc);
    }
    age = 20;
    Value values_stu2[3] = {{STRING, (void *) "3140104773"},
                            {STRING, (void *) "JunDong Xie"},
                            {INT,    &age}};
    if ((rc = qlm.Insert("students", 3, values_stu2))) {
        PrintError(rc);
    }
    smm.Print("students");
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

    RelAttr *selAttrs = new RelAttr[NSELATTRS];
    for (int m = 0; m < NSELATTRS; m++) {
        selAttrs[m].relName = NULL;
        selAttrs[m].attrName = new char[MAXNAME + 1];
    }
    memcpy(selAttrs[0].attrName, "departmentName", strlen("departmentName") + 1);
    memcpy(selAttrs[1].attrName, "buildYear", strlen("buildYear") + 1);
    memcpy(selAttrs[2].attrName, "budget", strlen("budget") + 1);
    memcpy(selAttrs[3].attrName, "id", strlen("id") + 1);
    memcpy(selAttrs[4].attrName, "name", strlen("name") + 1);
    memcpy(selAttrs[5].attrName, "age", strlen("age") + 1);
    memcpy(selAttrs[6].attrName, "id", strlen("id") + 1);
    memcpy(selAttrs[7].attrName, "course", strlen("course") + 1);

    selAttrs[3].relName = new char[MAXNAME + 1];
    memcpy(selAttrs[3].relName, "students", strlen("students") + 1);
    selAttrs[6].relName = new char[MAXNAME + 1];
    memcpy(selAttrs[6].relName, "takes", strlen("takes") + 1);

    int cmpYear = 1999;
    Value cmpValue = {STRING, (void *) "Computer Science"};
    Value cmpValue2 = {INT,
                       &cmpYear};
    Condition conds[NCONDS] = {{
                                       {NULL,          "departmentName"},
                                       GT_OP,
                                       0,
                                       {NULL, NULL},
                                       cmpValue,
                               },
                               {
                                       {"departments", "buildYear"},
                                       EQ_OP,
                                       0,
                                       {NULL, NULL},
                                       cmpValue2,
                               },
                               {
                                       {"students", "id"},
                                       EQ_OP,
                                       1,
                                       {"takes", "id",},
                                       {INT, NULL}
                               }};
    printf("\n\n");
//
    if ((rc = qlm.Select(NSELATTRS, selAttrs, NSELRELS, (const char *const *) relations, 0,
                         NULL))) {
        PrintError(rc);
    }
//    pfm.GetBufferManager()->PrintPageDescTable();
//    smm.Print("relcat");
//    smm.Print("attrcat");
    smm.CloseDb();
}