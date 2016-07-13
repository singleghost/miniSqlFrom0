#ifndef __MINISQL__
#define __MINISQL__

/*以页的方式组织管理文件，一页为４０９６ｋｂ，方便内存和硬盘进行页的交换*/
#define PAGE_SIZE 4096
#define PAGE_NUM_MAX 50
#define PF_FILE_Hdr 4096


typedef int RC; //函数返回的错误码，数字类型，　大于０为ｗａｒｎｉｎｇ，　小于０为ｅｒｒｏｒ，等于０表示没有错误
#define success 0

#endif 
