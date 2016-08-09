//
// Created by 谢俊东 on 16/8/3.
//

#include "lexAnalyser.h"
#include <iostream>

unsigned char downcaseTbl[256] =
        {
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
                45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
                60, 61, 62, 63, 64, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106,
                107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
                122, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104,
                105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
                120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
                135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
                150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
                165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
                180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
                195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
                210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
                225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
                240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
                255
        };

void LexAnalyser::getCmd(FILE *fin) {
    int i = 0;
    char c;
    while ((c = getc(fin)) != ';') {
        line[i++] = c;
        if (i >= MAXLINELEN) {
            break;
        }
    }
    line[i] = c;
    pos = 0;
}

//读取的token中关键词和identifier不区分大小写,只有字面量区分大小写
RC LexAnalyser::nextToken(Token &tok) {
    if (tok.value) {
        delete[] tok.value;
        tok.value = NULL;
    }  //释放空间
    char c;
    int beginPos;
    int keyword;
    int length;

    do {
        c = line[pos++];
    } while (c == ' ' || c == '\t' || c == '\n');    //忽略所有的空白字符
    beginPos = pos - 1;

    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        c = line[pos++];
        while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
            c = line[pos++];
        }
        pos--;  //rollback
        length = pos - beginPos;
        tok.value = (char *) calloc(1, length + 1);
        memcpy(tok.value, &line[beginPos], length);
        if ((keyword = Perfect_Hash::in_word_set(tok.value, pos - beginPos)) >= 0) {
            tok.kind = (KIND) keyword;
            free(tok.value);
            tok.value = NULL;
            return 0;
        } else {
            tok.kind = IDENTIFIER;
            tok.len = length;
            return 0;
        }
    } else if (c >= '0' && c <= '9') {
        c = line[pos++];
        while (c >= '0' && c <= '9') {
            c = line[pos++];
        }
        if (c == '.') {
            c = line[pos++];
            while (c >= '0' && c <= '9') {
                c = line[pos++];
            }
            if (c == ' ' || c == '\t' || c == '\n' || c == ';' || c == ',' || c == ')' || c == '(') {
                pos--;
                tok.kind = FLOATLIT;
                tok.len = pos - beginPos;
                tok.value = (char *) calloc(tok.len + 1, 1);
                memcpy(tok.value, &line[beginPos], tok.len);
                return 0;
            }
            return INVALID_INPUT;
        } else {
            if (c == ' ' || c == '\t' || c == '\n' || c == ';' || c == ',' || c == ')' || c == '(') {
                pos--;
                tok.kind = INTLIT;
                tok.len = pos - beginPos;
                tok.value = (char *) calloc(tok.len + 1, 1);
                memcpy(tok.value, &line[beginPos], tok.len);
                return 0;
            }
            return INVALID_INPUT;
        }
    } else {
        switch (c) {
            case '+':
            case '-':
                c = line[pos++];
                if (c >= '0' && c <= '9') {
                    c = line[pos++];
                    while (c >= '0' && c <= '9') {
                        c = line[pos++];
                    }
                    if (c == '.') {
                        c = line[pos++];
                        while (c >= '0' && c <= '9') {
                            c = line[pos++];
                        }
                        if (c == ' ' || c == '\t' || c == '\n' || c == ';' || c == ',' || c == ')' || c == '(') {
                            pos--;
                            tok.kind = FLOATLIT;
                            tok.len = pos - beginPos;
                            tok.value = (char *) calloc(tok.len + 1, 1);
                            memcpy(tok.value, &line[beginPos], tok.len);
                            return 0;
                        }
                        return INVALID_INPUT;
                    } else {
                        if (c == ' ' || c == '\t' || c == '\n' || c == ';' || c == ',' || c == ')' || c == '(') {
                            pos--;
                            tok.kind = INTLIT;
                            tok.len = pos - beginPos;
                            tok.value = (char *) calloc(tok.len + 1, 1);
                            memcpy(tok.value, &line[beginPos], tok.len);
                            return 0;
                        }
                        return INVALID_INPUT;
                    }
                } else return INVALID_INPUT;
            case '"':
                c = line[pos++];
                while (c != '"') {
                    if(c == '\0') return INVALID_INPUT;
                    c = line[pos++];
                }
                tok.kind = STRINGLIT;
                tok.len = pos - beginPos - 2;   //两个引号不需要算进去
                tok.value = (char *) calloc(tok.len + 1, 1);
                memcpy(tok.value, &line[beginPos + 1], tok.len);
                tok.value[tok.len] = '\0';  //以0字节结尾
                return 0;
            case '\'':
                c = line[pos++];
                while (c != '\'') {
                    if(c == '\0') return INVALID_INPUT;
                    c = line[pos++];
                }
                tok.kind = STRINGLIT;
                tok.len = pos - beginPos - 2;   //两个引号不需要算进去
                tok.value = (char *) calloc(tok.len + 1, 1);
                memcpy(tok.value, &line[beginPos + 1], tok.len);
                tok.value[tok.len] = '\0';  //以0字节结尾
                return 0;
            case '>':
                c = line[pos++];
                switch (c) {
                    case '=':
                        tok.kind = GE;
                        tok.value = NULL;
                        return 0;
                    default:
                        pos--;
                        tok.kind = GT;
                        tok.value = NULL;
                        return 0;
                }
            case '<':
                c = line[pos++];
                switch (c) {
                    case '>':
                        tok.kind = NE;
                        tok.value = NULL;
                        return 0;
                    case '=':
                        tok.kind = LE;
                        tok.value = NULL;
                        return 0;
                    default:
                        pos--;
                        tok.kind = LT;
                        tok.value = NULL;
                        return 0;
                }
            case '=':
                tok.kind = EQ;
                tok.value = NULL;
                return 0;
            case '!':
                c = line[pos++];
                if (c == '=') {
                    tok.kind = NE;
                    tok.value = NULL;
                    return 0;
                } else {
                    return INVALID_INPUT;
                }
            case ',':
                tok.kind = COMMA;
                tok.value = NULL;
                return 0;
            case ';':
                tok.kind = SEMICOLON;
                tok.value = NULL;
                return 0;
            case '(':
                tok.kind = LPAREN;
                tok.value = NULL;
                return 0;
            case ')':
                tok.kind = RPAREN;
                tok.value = NULL;
                return 0;
            case '.':
                tok.kind = DOT;
                tok.value = NULL;
                return 0;
            case '*':
                tok.kind = IDENTIFIER;
                tok.len = 1;
                tok.value = (char *) calloc(tok.len + 1, 1);
                tok.value[0] = '*';
                return 0;
        }
    }
    return INVALID_INPUT;
}