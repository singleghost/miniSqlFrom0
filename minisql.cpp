//
// Created by 谢俊东 on 16/7/22.
//
#include "minisql.h"
#include "SM/sm_manager.h"
#include "QL/ql.h"
#include "Parse/parser.h"

PF_Manager pfm;
RM_Manager rmm(pfm);
IX_Manager ixm(pfm);
SM_Manager smm(ixm, rmm);
QL_Manager qlm(smm, ixm, rmm);

int main(int argc, char *argv[]) {
//...
// initialize RedBase components
// open the database
    RC rc;
    char *dbname = argv[1];
    if (rc = smm.OpenDb(dbname)) {
        PrintError(rc);
    }
// call the parser
    RBparse(pfm, smm, qlm);
// close the database
    if (rc = smm.CloseDb()) {
        PrintError(rc);
    }
}