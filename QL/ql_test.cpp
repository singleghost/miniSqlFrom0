//
// Created by 谢俊东 on 16/7/29.
//

#include "ql.h"

int main()
{
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    IX_Manager ixm(pfm);
    SM_Manager smm(ixm, rmm);
    QL_Manager qlm(smm, ixm, rmm);

    printf("begin test\n\n\n");
    smm.OpenDb("ZJU");

    RelAttr *selAttrs = new RelAttr[3];
    selAttrs[0].attrName = new char[MAXNAME];
    selAttrs[1].attrName = new char[MAXNAME];
    selAttrs[2].attrName = new char[MAXNAME];
    memcpy(selAttrs[0].attrName, "departmentName", strlen("departmentName"));
    memcpy(selAttrs[1].attrName, "buildYear", strlen("buildYear"));
    memcpy(selAttrs[2].attrName, "budget", strlen("budget"));

    RC rc;
    char *relations[1] = { "departments" };
    if((rc = qlm.Select(3, selAttrs, 1, relations, 0, NULL))) {
        printf("rc=%d\n", rc);
    }
    smm.CloseDb();
}