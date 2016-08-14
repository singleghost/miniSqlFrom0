#ifndef __MINISQL__
#define __MINISQL__

#include <iostream>
using std::ostream;

/*以页的方式组织管理文件，一页为４０９６ｋｂ，方便内存和硬盘进行页的交换*/
#define PAGE_SIZE 4096
#define PF_FILE_Hdr 4096        //文件头大写
#define MAXSTRINGLEN 255   //attrType为字符串时最长的字符串大小
#define MAXNAME 24  //关系名和属性名的最大长度
#define MAXATTRS 40 //每个表最多能有40个属性值

#define YY_SKIP_YYWRAP 1
#define yywrap() 1
void yyerror(const char *);

typedef int RC; //函数返回的错误码，数字类型，　大于０为ｗａｒｎｉｎｇ，　小于０为ｅｒｒｏｒ，等于０表示没有错误

#define OK_RC         0    // OK_RC return code is guaranteed to always be 0

#define START_PF_ERR  (-1)
#define END_PF_ERR    (-100)
#define START_RM_ERR  (-101)
#define END_RM_ERR    (-200)
#define START_IX_ERR  (-201)
#define END_IX_ERR    (-300)
#define START_SM_ERR  (-301)
#define END_SM_ERR    (-400)
#define START_QL_ERR  (-401)
#define END_QL_ERR    (-500)
#define START_PARSER_ERR (-501)
#define END_PARSER_ERR (-600)

#define START_PF_WARN  1
#define END_PF_WARN    100
#define START_RM_WARN  101
#define END_RM_WARN    200
#define START_IX_WARN  201
#define END_IX_WARN    300
#define START_SM_WARN  301
#define END_SM_WARN    400
#define START_QL_WARN  401
#define END_QL_WARN    500
#define START_PARSER_WARN 501
#define END_PARSER_WARN 600

enum AttrType{
    INT,
    FLOAT,
    STRING
};  //字段的属性类型

enum CompOp {
    EQ_OP,
    LT_OP,
    LE_OP,
    GT_OP,
    GE_OP,
    NE_OP,
    NO_OP
};  //比较操作

enum ClientHint {
    NO_HINT
};

enum IX_pageType {
    LeafPage,
    InteriorPage
};

//struct RelAttr {
//    char *relName;     // relation name (may be NULL)
//    char *attrName;    // attribute name
//    friend ostream &operator<<(ostream &s, const RelAttr &ra);
//};
//
//struct Value {
//    AttrType type;     // type of value
//    void     *data;    // value
//    friend ostream &operator<<(ostream &s, const Value &v);
//};
//
//struct Condition {
//    RelAttr lhsAttr;      // left-hand side attribute
//    CompOp  op;           // comparison operator
//    int     bRhsIsAttr;   // TRUE if right-hand side is an attribute
//    //   and not a value
//    RelAttr rhsAttr;      // right-hand side attribute if bRhsIsAttr = TRUE
//    Value   rhsValue;     // right-hand side value if bRhsIsAttr = FALSE
//    friend ostream &operator<<(ostream &s, const Condition &c);
//};

//用来记录每一次查询所有属性的信息,多表join的查询时各个表的所有字段都包括进去
struct AttrInfoInRecord {   //字段与AttrCatTuple完全相同,offset字段含义不同
    char relName[MAXNAME + 1];
    char attrName[MAXNAME + 1];
    int offset;             //在整个查询record中的偏移
    AttrType attrType;
    int attrLength;
    int indexNo;
    int isPrimary;
};

//
// TRUE, FALSE and BOOLEAN
//
#ifndef BOOLEAN
typedef char Boolean;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif

#endif
