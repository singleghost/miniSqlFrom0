## minisql的CFG 上下文无关语法

G = (T, N, P, S)

S是唯一的开始符号, S = { S }

T 终结符 T = { select, from, where, insert, into, ID, values, LPAREN, RPAREN, update, set, delete, create, drop,
                table, index, load, help, print, eq, ne, gt, ge, lt, le, int, float, char, primary }

N 非终结符 N = { S,SELECT_CLAUSE, WHERE_CLAUSE, ATTRLIST,
TABLELIST, CONDLIST, LITERAL_LIST, ATTR_DEF_LIST, ATTR_DEF,
ATTRTYPE, ATTR, VALUE, COND, LITERAL,
                COMP }

P 产生式规则如下:

S -> SELECT_CLAUSE WHERE_CLAUSE;

S -> SELECT_CLAUSE;

SELECT_CLAUSE -> select ATTRLIST from TABLELIST

WHERE_CLAUSE -> where CONDLIST

S -> insert into ID values ( LITERAL_LIST );

S -> update ID set ID EQ Value WHERE_CLAUSE;

S -> delete from ID WHERE_CLAUSE;

S -> delete from ID;

S -> create table ID ( ATTR_DEF_LIST );

S -> drop table ID;

S -> create index ID ( ID );

S -> drop index ID ( ID );

S -> load ID ( STRING_LIT );

S -> help;

S -> help ID;

S -> print ID;

S -> set ID EQ STRING_LIT;

S -> exit;

ATTR_DEF_LIST -> ATTR_DEF, ATTR_DEF_LIST
                | ATTR_DEF

ATTR_DEF -> ID ATTRTYPE
            | ID ATTRTYPE primary

ATTRTYPE -> int
            | float
            | char ( INT_LIT )

ATTRLIST -> ATTR , ATTRLIST
          | ATTR

TABLELIST -> ID, TABLELIST
            | ID

ATTR -> ID . ID
        | ID

CONDLIST -> COND and CONDLIST
        | COND

COND -> ATTR COMP VALUE

VALUE -> ATTR
        | LITERAL

LITERAL_LIST -> LITERAL, LITERAL_LIST
                | LITERAL
LITERAL -> STRING_LIT
        |   INT_LIT
        |   FLOAT_LIT

COMP ->   eq
        | ne
        | ge
        | gt
        | lt
        | le
