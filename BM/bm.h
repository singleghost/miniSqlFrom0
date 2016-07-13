//
// Created by root on 16-6-5.
//

#ifndef BM_H
#define BM_H

#define MAX_BUFFER_POOL_SIZE 1000   //所能定义的最大buffer pool的大小

#define INVALID_SLOT -1             //不合法的slot,通常表示未找到



//返回码定义
#define BM_NO_FREE_BUF_WARNING -1   //没有空闲的缓冲区空间可以被分配了
#define SLOT_NOT_FOUND  -1          //slot未找到


//异常类
class page_exist_exception {};      //allocate page时如果page已经存在抛出的异常

#endif //BM_H
