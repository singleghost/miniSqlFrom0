//
// Created by 谢俊东 on 16/8/3.
//

#ifndef MINISQLFROM0_LEXANALYSER_H
#define MINISQLFROM0_LEXANALYSER_H

#include <cstdio>
#include "perfectHash.h"
#include "../minisql.h"

#define MAXLINELEN 200

#define INVALID_INPUT -1
enum KIND {
    NOP0, NOP1, ON, SET, LOAD, PRINT, SELECT, PRIMARY, INT_KEY, INTO, INDEX, INSERT, NOP2, AND, FROM, FLOAT_KEY, VALUES,NOP3, NOP4,
    DROP, WHERE, DELETE, NOP5, NOP6, CHAR, NOP7, CREATE, NOP8, NOP9, HELP, NOP10, UPDATE,//关键字
    INTLIT, STRINGLIT, FLOATLIT,            //字面量
    EQ, NE, GT, GE, LT, LE,                  //比较运算符
    IDENTIFIER,  //标识符
    COMMA, LPAREN, RPAREN, SEMICOLON,
    EOC, //end of command
};


struct Token {
    Token() { value = NULL; len = -1;}
    KIND kind;  //类型
    char *value;    //值
    unsigned int len;   //长度
    ~Token() { if(value) delete [] value; }
};

class LexAnalyser {
private:
    char line[MAXLINELEN];
    int pos;
public:
    void getCommand(FILE *fin);
    RC nextToken(Token &tok);

};

#endif //MINISQLFROM0_LEXANALYSER_H
