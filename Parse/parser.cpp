//
// Created by 谢俊东 on 16/8/8.
//

#include "parser.h"
#include "../QL/ql.h"

void PrintError(RC rc) {
    printf("rc=%d\n", rc);
    if(rc >= END_QL_ERR && rc <= START_QL_ERR) {
        QL_PrintError(rc);
    }
}
