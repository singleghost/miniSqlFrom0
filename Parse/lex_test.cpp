//
// Created by 谢俊东 on 16/8/3.
//

#include "SyntaxAnalyser.h"
#include <iostream>
using namespace std;

//ostream & operator<<(ostream &os, Token &token) {
//    os << "kind: " << token.kind << endl << "value: ";
//    if(token.kind == STRINGLIT) {
//        os << (char *)token.value;
//    } else if(token.kind == FLOATLIT) {
//
//    }
//}
PF_Manager pfm;
RM_Manager rmm(pfm);
IX_Manager ixm(pfm);
SM_Manager smm(ixm, rmm);
QL_Manager qlm(smm, ixm, rmm);
int main()
{
//    LexAnalyser lexer;
    RC rc;
//    Token tok;
//    while (true) {
//        printf("\nsql> ");
//        lexer.getCmd(stdin);
//        while ((rc = lexer.nextToken(tok)) != INVALID_INPUT) {
//            if (tok.kind == SEMICOLON) break;
//        }
//        if(rc != 0) {
//            printf("Error: wrong input\n");
//        }
//    }
    smm.OpenDb("ZJU");
    SyntaxAnalyser syntaxAnalyser(qlm, smm);
    while(true) {
        printf("\nsql> ");
        if((rc = syntaxAnalyser.parseCommand())) {
            PrintError(rc);
        }
    }
//    smm.CloseDb();
}