//
// Created by 谢俊东 on 16/8/6.
//

#ifndef MINISQLFROM0_SYNTAXANALYSER_H
#define MINISQLFROM0_SYNTAXANALYSER_H

#include "lexAnalyser.h"
#include "../minisql.h"

#define LEX_ERR -1
#define SYNTAX_ERR -2

class SyntaxAnalyser {
private:
    Token aheadToken;   //前看token
    bool bTokenBuffered;    //指示是否有前看token被缓冲
    LexAnalyser lexAnalyser;
    RC Parse_S();
    RC Parse_select_clause();
    RC Parse_where_clause();
    RC Parse_attrList();
    RC Parse_tableList();
    RC Parse_condList();
    RC Parse_literalList();
    RC Parse_attrDefList();
    RC Parse_attrDef();
    RC Parse_attrType();
    RC Parse_ATTR();
    RC Parse_VALUE();
    RC Parse_COND();
    RC Parse_literal();
    RC Parse_comp();
    void BufferToken(const Token &tok);
public:
    RC parseCommand();
    RC GetNextToken(Token &tok);
};


#endif //MINISQLFROM0_SYNTAXANALYSER_H
