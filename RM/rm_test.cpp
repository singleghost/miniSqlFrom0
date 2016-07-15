#include <iostream>
#include <zconf.h>
#include "rm.h"

#define RECORD_SIZE 4
using namespace std;
int main()
{
    cout << "start testing\n";

    PF_Manager pfm;
    RM_Manager rm_manager(pfm);

    rm_manager.CreateFile("test.sql", RECORD_SIZE);

    RM_FileHandler rm_fileHandler;
    if(rm_manager.OpenFile("test.sql", rm_fileHandler) == FILE_OPEN_ERROR) {
        cerr << "file open error\n";
    }
    char content[4];
    sprintf(content, "oooo");
    RID rid;

    for(int i = 0; i < 500; i++) {
        rm_fileHandler.InsertRec(content, rid);
    }

    RID del_rid;
    RM_Record new_rec;
    sprintf(content, "nnnn");
    int i, j;
    for(i = 100; i < 105; i++) {
        for (j = 0; j < 100; j++) {
            del_rid = RID(i, j);
            new_rec = RM_Record(content, del_rid, RECORD_SIZE);
            rm_fileHandler.UpdateRec(new_rec);
        }
    }

    RID s_rid;
    for(i = 0; i < 200; i++) {
        s_rid = RID(102, i);
        RM_Record rec;
        if(rm_fileHandler.GetRec(s_rid, rec) == RECORD_NOT_IN_USE) {
            cout << "record not in use!\n";
            continue;
        }
        write(0, rec.GetContent(), rec.GetRecordSize());
        cout << endl;
    }

    rm_manager.CloseFile(rm_fileHandler);

    cout << "reopen file \n";
    //重新打开文件,测试是否正常
    rm_manager.OpenFile("test.sql", rm_fileHandler);
    for(i = 0; i < 600; i++) {
        s_rid = RID(102, i);
        RM_Record rec;
        if(rm_fileHandler.GetRec(s_rid, rec) == RECORD_NOT_IN_USE) {
            cout << "record not in use!\n";
            continue;
        }
        write(0, rec.GetContent(), rec.GetRecordSize());
        cout << endl;
    }

    for(i = 100; i < 108; i++) {
        for (j = 0; j < 816; j++) {
            del_rid = RID(i, j);
            rm_fileHandler.DeleteRec(del_rid);
        }
    }

    rm_manager.CloseFile(rm_fileHandler);

    rm_manager.DestroyFile("test.sql");
}