//
// Created by 谢俊东 on 16/8/3.
//

#include "lexAnalyser.h"
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
int main()
{
    LexAnalyser lexer;
    RC rc;
    Token tok;
    while (true) {
        printf("\nsql> ");
        lexer.getCommand(stdin);
        while ((rc = lexer.nextToken(tok)) != INVALID_INPUT) {
            if (tok.kind == SEMICOLON) break;
        }
        if(rc != 0) {
            printf("Error: wrong input\n");
        }
    }
}