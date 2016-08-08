//
// Created by 谢俊东 on 16/8/6.
//

#ifndef MINISQLFROM0_SYNTAXANALYSER_H
#define MINISQLFROM0_SYNTAXANALYSER_H

#include "lexAnalyser.h"
#include "../minisql.h"
#include "../QL/ql.h"
#include <vector>

#define LEX_ERR -1
#define SYNTAX_ERR -2

struct Literal {
    int i;
    float f;
    string s;
    AttrType type;
};
class SyntaxAnalyser {
private:
    Token aheadTok;   //前看token
    bool bTokenBufed;    //指示是否有前看token被缓冲
    LexAnalyser lexer;
    //递归下降分析算法的所有非终结符的分析函数
    RC Parse_S();
    RC Parse_select_clause(vector<RelAttr> &attrs, vector<string> &tables);
    RC Parse_where_clause(vector<Condition> &conds);
    RC Parse_attrList(vector<RelAttr> &attrs);
    RC Parse_tableList(vector<string> &tables);
    RC Parse_condList(vector<Condition> &conds);
    RC Parse_literalList(vector<Value> &values);
    RC Parse_attrDefList(vector<AttrInfo> &attrDefs);
    RC Parse_attrDef(AttrInfo &attrInfo);
    RC Parse_attrType(AttrInfo &attrInfo);
    RC Parse_ATTR(RelAttr &relAttr);
    RC Parse_VALUE(RelAttr &rhsAttr, Value &rhsValue, int &isAttr);
    RC Parse_COND(Condition &cond);
    RC Parse_literal(Value &value);
    RC Parse_comp(CompOp &comp);
    void BufferToken(const Token &tok); //将token存储在缓冲区,即赋值给前看token
    RC GetNextToken(Token &tok);

    QL_Manager qlm;
    SM_Manager smm;

public:
    SyntaxAnalyser(QL_Manager &qlm, SM_Manager &smm) : qlm(qlm), smm(smm) {}
    RC parseCommand();
};


#endif //MINISQLFROM0_SYNTAXANALYSER_H
