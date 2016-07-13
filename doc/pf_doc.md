# 文件管理模块,按页来进行文件管理,用一个free list来进行页的分配和释放
# 每个文件都有文件头,占据一页
    struct fileHeader {
        int firstFreePage; //page free list的表头,第一个空闲页
        int numOfPages;         //文件总共有多少页，包括使用中和空闲的
    };  //文件头,为了方便,占据PAGE SIZE的空间

# 每一页都有页头

    struct pageheader {
        int nextfree;   //下一个free page
        int pagenum;        //页号
    };  //页头


用一个PF_Manager来进行所有文件的创建、打开、关闭、删除操作, PF_Manager封装了底层的bufferManager
打开文件后,用FileHandler作为句柄,来对文件进行各项操作。包括页的分配、释放,获取某一页,标志某页Dirty,将页写回磁盘、UnpinPage
获取某一页后,用PageHandler作为句柄,来对页进行操作。包括获取页号,获取页的内容(不包括页头的部分)

