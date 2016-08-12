//
// Created by 谢俊东 on 16/8/8.
//

#include "parser.h"
#include "../QL/ql.h"
#include "SyntaxAnalyser.h"

void PrintError(RC rc) {
    if(rc >= END_QL_ERR && rc <= START_QL_ERR) {
        QL_PrintError(rc);
    } else if(rc >= END_PF_ERR && rc <= START_PF_ERR) {
        PF_PrintError(rc);
    } else if(rc >= END_IX_ERR && rc <= START_IX_ERR) {
        IX_PrintError(rc);
    } else if(rc >= END_SM_ERR && rc <= START_SM_ERR) {
        SM_PrintError(rc);
    } else if(rc >= END_RM_ERR && rc <= START_RM_ERR) {
        RM_PrintError(rc);
    } else if(rc >= END_PARSER_ERR && rc <= START_PARSER_ERR) {
        PS_PrintError(rc);
    }
}
