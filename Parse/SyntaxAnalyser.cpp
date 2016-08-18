//
// Created by 谢俊东 on 16/8/6.
//

#include "SyntaxAnalyser.h"

RC SyntaxAnalyser::GetNextToken(Token &tok) {
    if (bTokenBufed) {
        bTokenBufed = false;
        tok = aheadTok;
        return 0;
    }
    else {
        if (lexer.nextToken(tok)) return LEX_ERR;
        return 0;
    }
}

void SyntaxAnalyser::BufferToken(const Token &tok) {
    bTokenBufed = true;
    aheadTok = tok;
}

RC SyntaxAnalyser::parseCommand() {
    RC rc;
    lexer.getCmd(stdin);
    if ((rc = Parse_S())) return rc;
    return 0;
}

RC SyntaxAnalyser::Parse_S() {
    Token tok;
    RC rc;
    int i;
    vector<RelAttr> attrs;
    vector<string> tables;
    vector<const char *> tableNames;
    RelAttr updAttr;
    RelAttr rhsAttr;
    int bRhsIsAttr;
    Value rhsValue;
    vector<Condition> conds;
    vector<Value> values;
    vector<AttrInfo> attrDefs;
    hasPrimary = false;

    if ((GetNextToken(tok))) return LEX_ERR;
    switch (tok.kind) {
        case SELECT :
            BufferToken(tok);
            if (Parse_select_clause(attrs, tables)) return SYNTAX_ERR;
            for (i = 0; i < tables.size(); i++) {
                tableNames.push_back(tables[i].c_str());
            }
            if ((GetNextToken(tok))) return LEX_ERR;
            if(attrs.size() == 1 && attrs[0].relName == nullptr && !strcmp(attrs[0].attrName, "*")) {
                //如果是通配符
                attrs.clear();
                smm.FillRelAttrs(tableNames.size(), &tableNames[0], attrs);
            }
            if (tok.kind == WHERE) {
                BufferToken(tok);
                if (Parse_where_clause(conds)) return SYNTAX_ERR;
                if (GetNextToken(tok)) return LEX_ERR;
                if (tok.kind == SEMICOLON) {
                    return qlm.Select(attrs.size(), &attrs[0], tableNames.size(), &tableNames[0], conds.size(), &conds[0]);
                }
                else return SYNTAX_ERR;
            } else if (tok.kind == SEMICOLON) {
                return qlm.Select(attrs.size(), &attrs[0], tableNames.size(), &tableNames[0], 0, NULL);
            }
            return SYNTAX_ERR;
        case INSERT:
            if ((GetNextToken(tok))) return LEX_ERR;
            if (tok.kind == INTO) {
                if ((GetNextToken(tok))) return LEX_ERR;
                if (tok.kind == IDENTIFIER) {
                    string relName = tok.value;
                    if ((GetNextToken(tok))) return LEX_ERR;
                    if (tok.kind == VALUES) {
                        if ((GetNextToken(tok))) return LEX_ERR;
                        if (tok.kind == LPAREN) {
                            if (Parse_literalList(values)) return SYNTAX_ERR;
                            if ((GetNextToken(tok))) return LEX_ERR;
                            if (tok.kind == RPAREN) {
                                if ((GetNextToken(tok))) return LEX_ERR;
                                if (tok.kind == SEMICOLON) {

                                    return qlm.Insert(relName.c_str(), values.size(), &values[0]);
                                }
                            }
                        }
                    }
                }
            }
            return SYNTAX_ERR;
        case UPDATE:
            if ((GetNextToken(tok))) return LEX_ERR;
            if (tok.kind == IDENTIFIER) {
                string relName = tok.value;
                if ((GetNextToken(tok))) return LEX_ERR;
                if (tok.kind == SET) {
                    if(Parse_ATTR(updAttr)) return SYNTAX_ERR;
                    if ((GetNextToken(tok))) return LEX_ERR;
                    if (tok.kind == EQ) {
                        if (Parse_VALUE(rhsAttr, rhsValue, bRhsIsAttr)) return SYNTAX_ERR;
                        if(GetNextToken(tok)) return LEX_ERR;
                        if(tok.kind == WHERE) {
                            BufferToken(tok);
                            if (Parse_where_clause(conds)) return SYNTAX_ERR;
                            if ((GetNextToken(tok))) return LEX_ERR;
                            if (tok.kind == SEMICOLON) {
                                return qlm.Update(relName.c_str(), updAttr, !bRhsIsAttr, rhsAttr, rhsValue, conds.size(),
                                           &conds[0]);
                            }
                        } else if(tok.kind == SEMICOLON){
                            return qlm.Update(relName.c_str(), updAttr, !bRhsIsAttr, rhsAttr, rhsValue, 0, nullptr);
                        }
                    }
                }
            }
            return SYNTAX_ERR;

        case DELETE:
            if ((GetNextToken(tok))) return LEX_ERR;
            if (tok.kind == FROM) {
                if ((GetNextToken(tok))) return LEX_ERR;
                if (tok.kind == IDENTIFIER) {
                    string relName = tok.value;
                    if ((GetNextToken(tok))) return LEX_ERR;
                    if (tok.kind == WHERE) {
                        BufferToken(tok);
                        if (Parse_where_clause(conds)) return SYNTAX_ERR;
                        if ((GetNextToken(tok))) return LEX_ERR;
                        if (tok.kind == SEMICOLON) {
                            return qlm.Delete(relName.c_str(), conds.size(), &conds[0]);
                        }
                    } else if (tok.kind == SEMICOLON) {
                        return qlm.Delete(relName.c_str(), 0, nullptr);
                    }

                }
            }
            return SYNTAX_ERR;

        case CREATE:
            if ((GetNextToken(tok))) return LEX_ERR;
            if (tok.kind == TABLE) {
                if ((GetNextToken(tok))) return LEX_ERR;
                if (tok.kind == IDENTIFIER) {
                    string relName = tok.value;
                    if ((GetNextToken(tok))) return LEX_ERR;
                    if (tok.kind == LPAREN) {
                        if (Parse_attrDefList(attrDefs)) return SYNTAX_ERR;
                        if (GetNextToken(tok)) return LEX_ERR;
                        if (tok.kind == RPAREN) {
                            if (GetNextToken(tok)) return LEX_ERR;
                            if (tok.kind == SEMICOLON) {
                                return smm.CreateTable(relName.c_str(), attrDefs.size(), &attrDefs[0]);
                            }
                        }
                    }
                }
                return SYNTAX_ERR;
            } else if (tok.kind == INDEX) {
                if (GetNextToken(tok)) return LEX_ERR;
                if (tok.kind == IDENTIFIER) {
                    string relName = tok.value;
                    if (GetNextToken(tok)) return LEX_ERR;
                    if (tok.kind == LPAREN) {
                        if (GetNextToken(tok)) return LEX_ERR;
                        if (tok.kind == IDENTIFIER) {
                            string attrName = tok.value;
                            if (GetNextToken(tok)) return LEX_ERR;
                            if (tok.kind == RPAREN) {
                                if (GetNextToken(tok)) return LEX_ERR;
                                if (tok.kind == SEMICOLON) {
                                    return smm.CreateIndex(relName.c_str(), attrName.c_str());
                                }

                            }
                        }
                    }
                }
                return SYNTAX_ERR;
            }
            return SYNTAX_ERR;
        case DROP:
            if (GetNextToken(tok)) return LEX_ERR;
            if (tok.kind == TABLE) {
                if (GetNextToken(tok)) return LEX_ERR;
                if (tok.kind == IDENTIFIER) {
                    string relName = tok.value;
                    if (GetNextToken(tok)) return LEX_ERR;
                    if (tok.kind == SEMICOLON) {
                        return smm.DropTable(relName.c_str());
                    }
                }
                return SYNTAX_ERR;
            } else if (tok.kind == INDEX) {
                if (GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == IDENTIFIER) {
                    string relName = tok.value;
                    if (GetNextToken(tok)) return LEX_ERR;
                    if (tok.kind == LPAREN) {
                        if (GetNextToken(tok)) return LEX_ERR;
                        if (tok.kind == IDENTIFIER) {
                            string attrName = tok.value;
                            if (GetNextToken(tok)) return LEX_ERR;
                            if (tok.kind == RPAREN) {
                                if (GetNextToken(tok)) return LEX_ERR;
                                if (tok.kind == SEMICOLON) {
                                    return smm.DropIndex(relName.c_str(), attrName.c_str());
                                }
                            }
                        }
                    }
                }
                return SYNTAX_ERR;
            }
            return SYNTAX_ERR;
        case LOAD:
            if (GetNextToken(tok)) return LEX_ERR;
            if (tok.kind == IDENTIFIER) {
                string relName = tok.value;
                if (GetNextToken(tok)) return LEX_ERR;
                if (tok.kind == LPAREN) {
                    if (GetNextToken(tok)) return LEX_ERR;
                    if (tok.kind == STRINGLIT) {
                        string loadfile = tok.value;
                        if (GetNextToken(tok)) return LEX_ERR;
                        if (tok.kind == RPAREN) {
                            if (GetNextToken(tok)) return LEX_ERR;
                            if (tok.kind == SEMICOLON) {
                                return smm.Load(relName.c_str(), loadfile.c_str());
                            }
                        }
                    }
                }
            }
            return SYNTAX_ERR;
        case HELP:
            if (GetNextToken(tok)) return LEX_ERR;
            if (tok.kind == IDENTIFIER) {
                string relName = tok.value;
                if (GetNextToken(tok)) return LEX_ERR;
                if (tok.kind == SEMICOLON) {
                    return smm.Help(relName.c_str());
                }
            } else if (tok.kind == SEMICOLON) {
                return smm.Help();
            }
            return SYNTAX_ERR;
        case PRINT:
            if (GetNextToken(tok)) return LEX_ERR;
            if (tok.kind == IDENTIFIER) {
                string relName = tok.value;
                if (GetNextToken(tok)) return LEX_ERR;
                if (tok.kind == SEMICOLON) {
                    return smm.Print(relName.c_str());
                }
            }
            return SYNTAX_ERR;
        case SET:
            if (GetNextToken(tok)) return LEX_ERR;
            if (tok.kind == IDENTIFIER) {
                if (GetNextToken(tok)) return LEX_ERR;
                if (tok.kind == EQ) {
                    if (GetNextToken(tok)) return LEX_ERR;
                    if (tok.kind == STRINGLIT) {
                        if (GetNextToken(tok)) return LEX_ERR;
                        if (tok.kind == SEMICOLON) return 0;
                    }
                }
            }
            return SYNTAX_ERR;
        case EXIT:
            if (GetNextToken(tok)) return LEX_ERR;
            if (tok.kind == SEMICOLON) {
                if((rc = smm.CloseDb())) PrintError(rc);
                exit(0);
            }
            return SYNTAX_ERR;
        default:
            return SYNTAX_ERR;

    }

}

RC SyntaxAnalyser::Parse_select_clause(vector<RelAttr> &attrs, vector<string> &tables) {
    RC rc;
    Token tok;
    if ((GetNextToken(tok))) return LEX_ERR;
    if (tok.kind == SELECT) {
        if (Parse_attrList(attrs)) return SYNTAX_ERR;
        if (GetNextToken(tok)) return LEX_ERR;
        if (tok.kind == FROM) {
            if (Parse_tableList(tables)) return SYNTAX_ERR;
            return 0;
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_where_clause(vector<Condition> &conds) {
    RC rc;
    Token tok;
    if (GetNextToken(tok)) return LEX_ERR;
    if (tok.kind == WHERE) {
        if (Parse_condList(conds)) return SYNTAX_ERR;
        return 0;
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_attrDefList(vector<AttrInfo> &attrDefs) {
    RC rc;
    Token tok;
    AttrInfo attrInfo;
    if ((rc = Parse_attrDef(attrInfo))) return rc;
    attrDefs.push_back(attrInfo);
    while (true) {
        if (GetNextToken(tok)) return LEX_ERR;
        if (tok.kind == COMMA) {
            if (Parse_attrDef(attrInfo)) return SYNTAX_ERR;
            attrDefs.push_back(attrInfo);
        } else {
            BufferToken(tok);
            return 0;
        }
    }
}

RC SyntaxAnalyser::Parse_attrDef(AttrInfo &attrInfo) {
    Token tok;
    if (GetNextToken(tok)) return LEX_ERR;
    if (tok.kind == IDENTIFIER) {
        attrInfo.attrName = new char[tok.len];
        strcpy(attrInfo.attrName, tok.value);
        if (Parse_attrType(attrInfo)) return SYNTAX_ERR;
        if (GetNextToken(tok)) return LEX_ERR;
        if (tok.kind == PRIMARY) {
            if(hasPrimary) return QL_PRIMARY_KEY_DUP;
            hasPrimary = true;
            attrInfo.bIsPrimary = 1;
            return 0;
        } else {
            attrInfo.bIsPrimary = 0;
            BufferToken(tok);
            return 0;
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_attrType(AttrInfo &attrInfo) {
    RC rc;
    Token tok;
    if (GetNextToken(tok)) return LEX_ERR;
    if (tok.kind == INT_KEY) {
        attrInfo.attrType = INT;
        attrInfo.attrLength = sizeof(int);
        return 0;
    } else if (tok.kind == FLOAT_KEY) {
        attrInfo.attrType = FLOAT;
        attrInfo.attrLength = sizeof(float);
        return 0;
    } else if (tok.kind == CHAR) {
        attrInfo.attrType = STRING;
        if (GetNextToken(tok)) return LEX_ERR;
        if (tok.kind == LPAREN) {
            if (GetNextToken(tok)) return LEX_ERR;
            if (tok.kind == INTLIT) {
                attrInfo.attrLength = atoi(tok.value);
                if (GetNextToken(tok)) return LEX_ERR;
                if (tok.kind == RPAREN) return 0;
            }
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_attrList(vector<RelAttr> &attrs) {
    RC rc;
    Token tok;
    RelAttr relAttr;
    if (Parse_ATTR(relAttr)) return SYNTAX_ERR;
    attrs.push_back(relAttr);
    while (true) {
        if (GetNextToken(tok)) return LEX_ERR;
        if (tok.kind == COMMA) {
            if (Parse_ATTR(relAttr)) return SYNTAX_ERR;
            attrs.push_back(relAttr);
        } else {
            BufferToken(tok);
            return 0;
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_tableList(vector<string> &tables) {
    RC rc;
    Token tok;
    if (GetNextToken(tok)) return LEX_ERR;
    if (tok.kind == IDENTIFIER) {
        tables.push_back(tok.value);
        while (true) {
            if (GetNextToken(tok)) return LEX_ERR;
            if (tok.kind == COMMA) {
                if (GetNextToken(tok)) return LEX_ERR;
                if (tok.kind == IDENTIFIER) {
                    tables.push_back(tok.value);
                    continue;
                }
            } else {
                BufferToken(tok);
                return 0;
            }
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_ATTR(RelAttr &relAttr) {
    RC rc;
    Token tok;
    if (GetNextToken(tok)) return LEX_ERR;
    if (tok.kind == IDENTIFIER) {
        string name = tok.value;

        if (GetNextToken(tok)) return LEX_ERR;
        if (tok.kind == DOT) {
            if (GetNextToken(tok)) return LEX_ERR;
            if (tok.kind == IDENTIFIER) {
                relAttr = RelAttr(name.c_str(), tok.value);
                return 0;
            }
        } else {
            relAttr = RelAttr(NULL, name.c_str());
            BufferToken(tok);
            return 0;
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_condList(vector<Condition> &conds) {
    RC rc;
    Token tok;
    Condition cond;
    if (Parse_COND(cond)) return SYNTAX_ERR;
    conds.push_back(cond);
    while (true) {
        if (GetNextToken(tok)) return LEX_ERR;
        if (tok.kind == AND) {
            if (Parse_COND(cond)) return SYNTAX_ERR;
            conds.push_back(cond);
        } else {
            BufferToken(tok);
            return 0;
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_COND(Condition &cond) {
    if (Parse_ATTR(cond.lhsAttr)) return SYNTAX_ERR;
    if (Parse_comp(cond.op)) return SYNTAX_ERR;
    if (Parse_VALUE(cond.rhsAttr, cond.rhsValue, cond.bRhsIsAttr)) return SYNTAX_ERR;
    return 0;
}

RC SyntaxAnalyser::Parse_VALUE(RelAttr &rhsAttr, Value &rhsValue, int &isAttr) {
    Token tok;
    if (GetNextToken(tok)) return LEX_ERR;
    isAttr = 0;
    switch (tok.kind) {
        case STRINGLIT:
            rhsValue.type = STRING;
            rhsValue.data = new char[tok.len + 1];
            strcpy(rhsValue.data, tok.value);
            return 0;
        case INTLIT:
            rhsValue.type = INT;
            rhsValue.data = new char[sizeof(int)];
            sscanf(tok.value, "%d", (int *)rhsValue.data);
            return 0;
        case FLOATLIT:
            rhsValue.type = FLOAT;
            rhsValue.data = new char[sizeof(float)];
            sscanf(tok.value, "%f", (float *)rhsValue.data);
            return 0;
        default:
            isAttr = 1;
            BufferToken(tok);
            if (Parse_ATTR(rhsAttr)) return SYNTAX_ERR;
            return 0;
    }
}

RC SyntaxAnalyser::Parse_comp(CompOp &comp) {
    RC rc;
    Token tok;
    if (GetNextToken(tok)) return LEX_ERR;
    switch (tok.kind) {
        case EQ:
            comp = EQ_OP;
            return 0;
        case NE:
            comp = NE_OP;
            return 0;
        case GE:
            comp = GE_OP;
            return 0;
        case GT:
            comp = GT_OP;
            return 0;
        case LT:
            comp = LT_OP;
            return 0;
        case LE:
            comp = LE_OP;
            return 0;
        default:
            return SYNTAX_ERR;
    }
}

RC SyntaxAnalyser::Parse_literal(Value &value) {
    RC rc;
    Token tok;
    if (GetNextToken(tok)) return LEX_ERR;
    if (value.data) delete[] value.data;
    switch (tok.kind) {
        case STRINGLIT:
            value.type = STRING;
            value.data = new char[tok.len + 1];
            strcpy((char *) value.data, tok.value);
            return 0;
        case INTLIT:
            value.type = INT;
            value.data = new char[sizeof(int)];
            sscanf(tok.value, "%d", (int *)value.data);
            return 0;
        case FLOATLIT:
            value.type = FLOAT;
            value.data = new char[sizeof(float)];
            sscanf(tok.value, "%f", (float *)value.data);
            return 0;
        default:
            return SYNTAX_ERR;
    }
}

RC SyntaxAnalyser::Parse_literalList(vector<Value> &values) {
    RC rc;
    Token tok;
    Value value;
    if (Parse_literal(value)) return SYNTAX_ERR;
    values.push_back(value);
    while (true) {
        if (GetNextToken(tok)) return LEX_ERR;
        if (tok.kind == COMMA) {
            if (Parse_literal(value)) return SYNTAX_ERR;
            values.push_back(value);
        } else {
            BufferToken(tok);
            return 0;
        }
    }
    return SYNTAX_ERR;
}

const char *ps_error_msg[] = { "Parser: unrecognizable token", "Parser: syntax check error", "define too many primary keys"};
void PS_PrintError(RC rc) {

    printf("Error: %s\n", ps_error_msg[START_PARSER_ERR - 1 - rc]);
}
