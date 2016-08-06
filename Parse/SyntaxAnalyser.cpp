//
// Created by 谢俊东 on 16/8/6.
//

#include "SyntaxAnalyser.h"

RC SyntaxAnalyser::GetNextToken(Token &tok) {
    if(bTokenBuffered) {
        bTokenBuffered = false;
        tok = aheadToken;
        return 0;
    }
    else {
        if( lexAnalyser.nextToken(tok)) return LEX_ERR;
        return 0;
    }
}

void SyntaxAnalyser::BufferToken(const Token &tok) {
    bTokenBuffered = true;
    aheadToken = tok;
}

RC SyntaxAnalyser::parseCommand() {
    lexAnalyser.getCommand(stdin);
    if(Parse_S()) return SYNTAX_ERR;
    return 0;
}

RC SyntaxAnalyser::Parse_S() {
    Token tok;
    RC rc;
    if( (GetNextToken(tok))) return LEX_ERR;
    switch (tok.kind) {
        case SELECT :
            BufferToken(tok);
            if( Parse_select_clause()) return SYNTAX_ERR;
            if( (GetNextToken(tok))) return LEX_ERR;
            if(tok.kind == WHERE) {
                BufferToken(tok);
                if( Parse_where_clause()) return SYNTAX_ERR;
                if( GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == SEMICOLON) return 0;
                else return SYNTAX_ERR;
            } else if(tok.kind == SEMICOLON) {
                return 0;
            }
            return SYNTAX_ERR;
        case INSERT:
            if( (GetNextToken(tok))) return LEX_ERR;
            if(tok.kind == INTO) {
                if( (GetNextToken(tok))) return LEX_ERR;
                if(tok.kind == IDENTIFIER) {
                    if( (GetNextToken(tok))) return LEX_ERR;
                    if(tok.kind == VALUES) {
                        if( (GetNextToken(tok))) return LEX_ERR;
                        if(tok.kind == LPAREN) {
                            if(Parse_literalList()) return SYNTAX_ERR;
                            if( (GetNextToken(tok))) return LEX_ERR;
                            if(tok.kind == RPAREN) {
                                if( (GetNextToken(tok))) return LEX_ERR;
                                if(tok.kind == SEMICOLON) {
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
            return SYNTAX_ERR;
        case UPDATE:
            if( (GetNextToken(tok))) return LEX_ERR;
            if(tok.kind == IDENTIFIER) {
                if( (GetNextToken(tok))) return LEX_ERR;
                if(tok.kind == SET) {
                    if( (GetNextToken(tok))) return LEX_ERR;
                    if(tok.kind == IDENTIFIER) {
                        if( (GetNextToken(tok))) return LEX_ERR;
                        if(tok.kind == EQ) {
                            if(Parse_VALUE()) return SYNTAX_ERR;
                            if(Parse_where_clause()) return SYNTAX_ERR;
                            if( (GetNextToken(tok))) return LEX_ERR;
                            if(tok.kind == SEMICOLON) return 0;
                        }
                    }
                }
            }
            return SYNTAX_ERR;

        case DELETE:
            if( (GetNextToken(tok))) return LEX_ERR;
            if(tok.kind == FROM) {
                if( (GetNextToken(tok))) return LEX_ERR;
                if(tok.kind == IDENTIFIER) {
                    if( (GetNextToken(tok))) return LEX_ERR;
                    if(tok.kind == WHERE) {
                        BufferToken(tok);
                        if(Parse_where_clause()) return SYNTAX_ERR;
                        if( (GetNextToken(tok))) return LEX_ERR;
                        if(tok.kind == SEMICOLON) return 0;
                    } else if(tok.kind == SEMICOLON) return 0;

                }
            }
            return SYNTAX_ERR;

        case CREATE:
            if( (GetNextToken(tok))) return LEX_ERR;
            if(tok.kind == TABLE) {
                if( (GetNextToken(tok))) return LEX_ERR;
                if(tok.kind == IDENTIFIER) {
                    if( (GetNextToken(tok))) return LEX_ERR;
                    if(tok.kind == LPAREN) {
                        if(Parse_attrDefList()) return SYNTAX_ERR;
                        if(GetNextToken(tok)) return LEX_ERR;
                        if(tok.kind == RPAREN) {
                            if(GetNextToken(tok)) return LEX_ERR;
                            if(tok.kind == SEMICOLON) return 0;
                        }
                    }
                }
                return SYNTAX_ERR;
            } else if(tok.kind == INDEX) {
                if(GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == IDENTIFIER) {
                    if(GetNextToken(tok)) return LEX_ERR;
                    if(tok.kind == LPAREN) {
                        if(GetNextToken(tok)) return LEX_ERR;
                        if(tok.kind == IDENTIFIER) {
                            if(GetNextToken(tok)) return LEX_ERR;
                            if(tok.kind == RPAREN) {
                                if(GetNextToken(tok)) return LEX_ERR;
                                if(tok.kind == SEMICOLON) return 0;
                            }
                        }
                    }
                }
                return SYNTAX_ERR;
            }
            return SYNTAX_ERR;
        case DROP:
            if(GetNextToken(tok)) return LEX_ERR;
            if(tok.kind == TABLE) {
                if(GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == IDENTIFIER) {
                    if(GetNextToken(tok)) return LEX_ERR;
                    if(tok.kind == SEMICOLON) return 0;
                }
                return SYNTAX_ERR;
            } else if(tok.kind == INDEX) {
                if(GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == LPAREN) {
                    if(GetNextToken(tok)) return LEX_ERR;
                    if(tok.kind == IDENTIFIER) {
                        if(GetNextToken(tok)) return LEX_ERR;
                        if(tok.kind == RPAREN) {
                            if(GetNextToken(tok)) return LEX_ERR;
                            if(tok.kind == SEMICOLON) return 0;
                        }
                    }
                }
                return SYNTAX_ERR;
            }
            return SYNTAX_ERR;
        case LOAD:
            if(GetNextToken(tok)) return LEX_ERR;
            if(tok.kind == IDENTIFIER) {
                if(GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == LPAREN) {
                    if(GetNextToken(tok)) return LEX_ERR;
                    if(tok.kind == STRINGLIT) {
                        if(GetNextToken(tok)) return LEX_ERR;
                        if(tok.kind == RPAREN) {
                            if(GetNextToken(tok)) return LEX_ERR;
                            if(tok.kind == SEMICOLON) return 0;
                        }
                    }
                }
            }
            return SYNTAX_ERR;
        case HELP:
            if(GetNextToken(tok)) return LEX_ERR;
            if(tok.kind == IDENTIFIER) {
                if(GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == SEMICOLON) return 0;
            } else if(tok.kind == SEMICOLON) return 0;
            return SYNTAX_ERR;
        case PRINT:
            if(GetNextToken(tok)) return LEX_ERR;
            if(tok.kind == IDENTIFIER) {
                if(GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == SEMICOLON) return 0;
            }
            return SYNTAX_ERR;
        case SET:
            if(GetNextToken(tok)) return LEX_ERR;
            if(tok.kind == IDENTIFIER) {
                if(GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == EQ) {
                    if(GetNextToken(tok)) return LEX_ERR;
                    if(tok.kind == STRINGLIT) {
                        if(GetNextToken(tok)) return LEX_ERR;
                        if(tok.kind == SEMICOLON) return 0;
                    }
                }
            }
            return SYNTAX_ERR;
        case EXIT:
            if(GetNextToken(tok)) return LEX_ERR;
            if(tok.kind == SEMICOLON) {
                exit(0);
            }
            return SYNTAX_ERR;
        default:
            return SYNTAX_ERR;

    }

}

RC SyntaxAnalyser::Parse_select_clause() {
    RC rc;
    Token tok;
    if((GetNextToken(tok))) return LEX_ERR;
    if(tok.kind == SELECT) {
        if(Parse_attrList()) return SYNTAX_ERR;
        if(GetNextToken(tok)) return LEX_ERR;
        if(tok.kind == FROM) {
            if(Parse_tableList()) return SYNTAX_ERR;
            return 0;
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_where_clause() {
    RC rc;
    Token tok;
    if(GetNextToken(tok)) return LEX_ERR;
    if(tok.kind == WHERE) {
        if(Parse_condList()) return SYNTAX_ERR;
        return 0;
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_attrDefList() {
    RC rc;
    Token tok;
    if(Parse_attrDef()) return SYNTAX_ERR;
    while(true) {
        if(GetNextToken(tok)) return LEX_ERR;
        if(tok.kind == COMMA) {
            if(Parse_attrDef()) return SYNTAX_ERR;
        } else {
            BufferToken(tok);
            return 0;
        }
    }
}

RC SyntaxAnalyser::Parse_attrDef() {
    RC rc;
    Token tok;
    if(GetNextToken(tok)) return LEX_ERR;
    if(tok.kind == IDENTIFIER) {
        if(Parse_attrType()) return SYNTAX_ERR;
        if(GetNextToken(tok)) return LEX_ERR;
        if(tok.kind == PRIMARY) {
            return 0;
        } else {
            BufferToken(tok);
            return 0;
        }
    }
}

RC SyntaxAnalyser::Parse_attrType() {
    RC rc;
    Token tok;
    if(GetNextToken(tok)) return LEX_ERR;
    if(tok.kind == INT_KEY) {
        return 0;
    } else if(tok.kind == FLOAT_KEY) {
        return 0;
    } else if(tok.kind == CHAR) {
        if(GetNextToken(tok)) return LEX_ERR;
        if(tok.kind == LPAREN) {
            if(GetNextToken(tok)) return LEX_ERR;
            if(tok.kind == INTLIT) {
                if(GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == RPAREN) return 0;
            }
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_attrList() {
    RC rc;
    Token tok;
    if(Parse_ATTR()) return SYNTAX_ERR;
    while(true) {
        if(GetNextToken(tok)) return LEX_ERR;
        if(tok.kind == COMMA) {
            if(Parse_ATTR()) return SYNTAX_ERR;
        } else {
            BufferToken(tok);
            return 0;
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_tableList() {
    RC rc;
    Token tok;
    if(GetNextToken(tok)) return LEX_ERR;
    if(tok.kind == IDENTIFIER) {
        while(true) {
            if(GetNextToken(tok)) return LEX_ERR;
            if(tok.kind == COMMA) {
                if(GetNextToken(tok)) return LEX_ERR;
                if(tok.kind == IDENTIFIER) continue;
            } else {
                BufferToken(tok);
                return 0;
            }
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_ATTR() {
    RC rc;
    Token tok;
    if(GetNextToken(tok)) return LEX_ERR;
    if(tok.kind == IDENTIFIER) {
        if(GetNextToken(tok)) return LEX_ERR;
        if(tok.kind == DOT) {
            if(GetNextToken(tok)) return LEX_ERR;
            if(tok.kind == IDENTIFIER) {
                return 0;
            }
        } else {
            BufferToken(tok);
            return 0;
        }
    }
}

RC SyntaxAnalyser::Parse_condList() {
    RC rc;
    Token tok;
    if(Parse_COND()) return SYNTAX_ERR;
    while(true) {
        if(GetNextToken(tok)) return LEX_ERR;
        if(tok.kind == AND) {
            if(Parse_COND()) return SYNTAX_ERR;
        } else {
            BufferToken(tok);
            return 0;
        }
    }
    return SYNTAX_ERR;
}

RC SyntaxAnalyser::Parse_COND() {
    RC rc;
    Token tok;
    if(Parse_ATTR()) return SYNTAX_ERR;
    if(Parse_comp()) return SYNTAX_ERR;
    if(Parse_VALUE()) return SYNTAX_ERR;
    return 0;
}

RC SyntaxAnalyser::Parse_VALUE() {
    RC rc;
    Token tok;
    if(GetNextToken(tok)) return LEX_ERR;
    switch (tok.kind) {
        case STRINGLIT:
        case INTLIT:
        case FLOATLIT:
            return 0;
        default:
            BufferToken(tok);
            if(Parse_ATTR()) return SYNTAX_ERR;
            return 0;
    }
}

RC SyntaxAnalyser::Parse_comp() {
    RC rc;
    Token tok;
    if(GetNextToken(tok)) return LEX_ERR;
    switch (tok.kind) {
        case EQ:case NE:case GE:case GT:case LT:case LE:
            return 0;
        default:
            return SYNTAX_ERR;
    }
}

RC SyntaxAnalyser::Parse_literal() {
    RC rc;
    Token tok;
    if(GetNextToken(tok)) return LEX_ERR;
    switch (tok.kind) {
        case STRINGLIT:case INTLIT:case FLOATLIT:
            return 0;
        default:
            return SYNTAX_ERR;
    }
}

RC SyntaxAnalyser::Parse_literalList() {
    RC rc;
    Token tok;
    if(Parse_literal()) return SYNTAX_ERR;
    while(true) {
        if(GetNextToken(tok)) return LEX_ERR;
        if(tok.kind == COMMA) {
            if(Parse_literal()) return SYNTAX_ERR;
        } else {
            BufferToken(tok);
            return 0;
        }
    }
    return SYNTAX_ERR;
}