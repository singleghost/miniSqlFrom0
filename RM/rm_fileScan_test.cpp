//
// Created by 谢俊东 on 16/7/15.
//

#include "rm.h"
#include <iostream>
#define FILENAME "testScan.sql"

using std::cout;
using std::cerr;
using std::endl;

typedef struct tuple{
    char name[16];
    int age;
    float weight;
} tuple;

int main()
{
    PF_Manager pfm;
    RM_Manager rm_manager(pfm);
    rm_manager.CreateFile(FILENAME, sizeof(tuple));

    RM_FileHandler rm_fileHandler;
    if(rm_manager.OpenFile(FILENAME, rm_fileHandler) == FILE_OPEN_ERROR) {
        cerr << "file open error\n";
    }
    tuple content = { "xiejundong", 20, 70.5};
    RID rid;

    for(int i = 0; i < 900; i++) {
        rm_fileHandler.InsertRec((char *) &content, rid);
    }
    content = { "jinzijie", 21, 88.12};
    for(int i = 0; i < 900; i++) {
        rm_fileHandler.InsertRec((char *) &content, rid);
    }

    //开始扫描
    RM_FileScan rm_fileScan;

    char cmp_value[16];
    sprintf(cmp_value, "jinzijie");
//    float cmp_value = 88.12;
    rm_fileScan.OpenScan(rm_fileHandler, STRING, 16, 0, EQ_OP, &cmp_value);

    RM_Record rec;
    while(true) {
        if( rm_fileScan.GetNextRec(rec) == RM_EOF ) break;
        RID s_rid = rec.GetRid();
        cout << "page: " << s_rid.getPageNum() << "\tslot:" << s_rid.getSlot() << endl;
        if(s_rid.getSlot() == 815) {

        }
    }

    rm_fileScan.CloseScan();
    if( rm_fileScan.GetNextRec(rec) == -1) {
        printf("file scan has been closed\n");
    }
    //clean
    rm_manager.CloseFile(rm_fileHandler);
    rm_manager.DestroyFile(FILENAME);
}

