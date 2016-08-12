//
// Created by 谢俊东 on 16/8/3.
//

#include "SyntaxAnalyser.h"
#include <iostream>
using namespace std;

PF_Manager pfm;
RM_Manager rmm(pfm);
IX_Manager ixm(pfm);
SM_Manager smm(ixm, rmm);
QL_Manager qlm(smm, ixm, rmm);
SyntaxAnalyser syntaxAnalyser(qlm, smm);
int main()
{
    RC rc;
    smm.OpenDb("ZJU");
    while(true) {
        printf("\nsql> ");
        if((rc = syntaxAnalyser.parseCommand())) {
            PrintError(rc);
        }
    }
}