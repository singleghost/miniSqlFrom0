#ifndef __MINISQL__
#define __MINISQL__

#include <iostream>
using std::ostream;
/*以页的方式组织管理文件，一页为４０９６ｋｂ，方便内存和硬盘进行页的交换*/
#define PAGE_SIZE 4096
#define PF_FILE_Hdr 4096        //文件头大写
#define MAX_STRING_LENGTH 255   //attrType为字符串时最长的字符串大小
#define MAXNAME 24  //关系名和属性名的最大长度
#define MAXATTRS 40 //每个表最多能有40个属性值

typedef int RC; //函数返回的错误码，数字类型，　大于０为ｗａｒｎｉｎｇ，　小于０为ｅｒｒｏｒ，等于０表示没有错误
#define success 0


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

struct RelAttr {
    char *relName;     // relation name (may be NULL)
    char *attrName;    // attribute name
    friend ostream &operator<<(ostream &s, const RelAttr &ra);
};

struct Value {
    AttrType type;     // type of value
    void     *data;    // value
    friend ostream &operator<<(ostream &s, const Value &v);
};

struct Condition {
    RelAttr lhsAttr;      // left-hand side attribute
    CompOp  op;           // comparison operator
    int     bRhsIsAttr;   // TRUE if right-hand side is an attribute
    //   and not a value
    RelAttr rhsAttr;      // right-hand side attribute if bRhsIsAttr = TRUE
    Value   rhsValue;     // right-hand side value if bRhsIsAttr = FALSE
    friend ostream &operator<<(ostream &s, const Condition &c);
};

//用来记录每一次查询所有属性的信息,多表join的查询时各个表的所有字段都包括进去
struct AttrInfoInRecord {   //字段与AttrCatTuple完全相同,offset字段含义不同
    char relName[MAXNAME + 1];
    char attrName[MAXNAME + 1];
    int offset;             //在整个查询record中的偏移
    AttrType attrType;
    int attrLength;
    int indexNo;
};
#endif
