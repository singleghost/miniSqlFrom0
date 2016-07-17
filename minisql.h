#ifndef __MINISQL__
#define __MINISQL__

/*以页的方式组织管理文件，一页为４０９６ｋｂ，方便内存和硬盘进行页的交换*/
#define PAGE_SIZE 4096
#define PF_FILE_Hdr 4096        //文件头大写
#define MAX_STRING_LENGTH 255   //attrType为字符串时最长的字符串大小

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

#endif 
