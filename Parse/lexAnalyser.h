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
    NOP0,
    NOP1,
    ON,
    SET,
    LOAD,
    PRINT,
    SELECT,
    PRIMARY,
    INT_KEY,
    INTO,
    INDEX,
    INSERT,
    NOP2,
    AND,
    FROM,
    FLOAT_KEY,
    VALUES,
    NOP3,
    NOP4,
    DROP,
    WHERE,
    DELETE,
    NOP5,
    NOP6,
    CHAR,
    TABLE,
    CREATE,
    NOP8,
    NOP9,
    HELP,
    NOP10,
    UPDATE,
    NOP11,
    NOP12,
    EXIT, //关键字
    INTLIT,
    STRINGLIT,
    FLOATLIT,            //字面量
    EQ,
    NE,
    GT,
    GE,
    LT,
    LE,                  //比较运算符
    IDENTIFIER,  //标识符
    COMMA,
    LPAREN,
    RPAREN,
    SEMICOLON,
    DOT,  // , ( ) ; .
    EOC, //end of command
};


class Token {
public:
    KIND kind; //类型
    char *value; //值
    unsigned int len; //长度

    Token() {
        value = NULL;
        len = -1;
    }

    ~Token() { if (value) delete[] value; }

    Token(const Token &tok) {
        if (tok.value) {
            value = new char[tok.len + 1];
            strcpy(value, tok.value);
        } else {
            value = tok.value;
        }
        len = tok.len;
        kind = tok.kind;
    }

    Token &operator=(const Token &tok) {
        if (this == &tok) return *this;
        if (value) delete[] value;
        if (tok.value) {
            value = new char[tok.len + 1];
            strcpy(value, tok.value);
        }
        else value = tok.value;
        len = tok.len;
        kind = tok.kind;

        return *this;
    }
};

class LexAnalyser {
private:
    char line[MAXLINELEN];
    int pos;
public:
    void getCmd(FILE *fin);

    RC nextToken(Token &tok);

};

#endif //MINISQLFROM0_LEXANALYSER_H
