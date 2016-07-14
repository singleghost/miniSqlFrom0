RM模块的文件头,存放位置紧邻着PF模块的文件头
    struct RM_FileHeader {
        int RecordSize;
        int nMaxRecordsPerPage;
        int numOfPages;         //PF文件头和RM文件头不算,总共有多少页
    };

RM模块的页头,存放第一个空闲的record位置,用free list的方式来管理record的增删改查,每个record前都有额外的一个字节(不算在record size里)
来表示当前record空间的状态:
1 表示正在被使用
0 表示没有被使用
如果正在被使用,那么后面record size个字节存放的是record的实际内容
如果没有被使用,那么后面的4个字节,存放的是free list的内容(>=0 表示下一个free list表项, -1表示free list末尾)

    struct RM_PageHeader {
        int firstFreeRec;   //第一个空闲的record的位置
        int NumOfRecords;   //当前页共有多少Record, 不包括空闲的
    };

RM_FileHandle类用来操作文件中的记录。为了操作文件中的记录,client创建一个RM_FileHandle 实例并传递给RM_Manager
的OpenFile函数。

    class RM_FileHandle {
      public:
           RM_FileHandle  ();                                  // Constructor
           ~RM_FileHandle ();                                  // Destructor
        RC GetRec         (const RID &rid, RM_Record &rec) const;
                                                               // Get a record
        RC InsertRec      (const char *pData, RID &rid);       // Insert a new record,
                                                               //   return record id
        RC DeleteRec      (const RID &rid);                    // Delete a record
        RC UpdateRec      (const RM_Record &rec);              // Update a record
        RC ForcePages     (PageNum pageNum = ALL_PAGES) const; // Write dirty page(s)
                                                               //   to disk
    };

RID类用pageNum和slotNum来标识每一个record。 RM_Record则包含了RID以及record的具体内容的拷贝