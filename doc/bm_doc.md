## BufferManager

    struct bufferPageDesc{
        char *page_addr;
        int next;
        int prev;
        int pinCount;
        bool isdirty;
        int pageNum;
        int fd;
    };

维护一个定长数组，数组中的每一项存储一个页的地址，同时维护一个空闲链表，一个使用中链表， 用来进行页的分配和释放
***

next 和 prev 用来维持一个双向链表，但每项既可以在free list中，也可以在used list中。
用firstFree变量来记录free list的头，用firstUsed和lastUsed来记录used list的头和尾.