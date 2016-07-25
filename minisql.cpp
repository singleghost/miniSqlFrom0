//
// Created by 谢俊东 on 16/7/22.
//

int main(...) {
//...
// initialize RedBase components
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    IX_Manager ixm(pfm);
    SM_Manager smm(ixm, rmm);
    QL_Manager qlm(smm, ixm, rmm);
// open the database
    if (rc = smm.OpenDb(dbname)) ...
// call the parser
    RBparse(pfm, smm, qlm);
// close the database
    if (rc = smm.CloseDb()) ...
}