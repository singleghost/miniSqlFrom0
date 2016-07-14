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
    cout << rm_fileHandler.getNMaxRecordPerPage() << endl;
    cout << rm_fileHandler.getRecordSize() << endl;
    char content[4];
    RID rid;
    sprintf(content, "0001");

    for(int i = 0; i < 900; i++) {
        rm_fileHandler.InsertRec(content, rid);
    }

    RID del_rid;
    RM_Record new_rec;
    sprintf(content, "0010");
    int j;
    for(j = 1; j < 80; j++){
        del_rid = RID(1, j);
        new_rec = RM_Record(content, del_rid, RECORD_SIZE);
        rm_fileHandler.UpdateRec(new_rec);
    }

    RID s_rid;
    int i;
    for(i = 0; i < 80; i++) {
        s_rid = RID(1, i);
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
    for(i = 0; i < 80; i++) {
        s_rid = RID(1, i);
        RM_Record rec;
        if(rm_fileHandler.GetRec(s_rid, rec) == RECORD_NOT_IN_USE) {
            cout << "record not in use!\n";
            continue;
        }
        write(0, rec.GetContent(), rec.GetRecordSize());
        cout << endl;
    }

    for(j = 0; j < 816; j++){
        del_rid = RID(0, j);
//        new_rec = RM_Record(content, del_rid, RECORD_SIZE);
        rm_fileHandler.DeleteRec(del_rid);
    }
    rm_fileHandler.ForcePages();
    for(int j = 0; j < 800; j++) {
        rm_fileHandler.InsertRec(content, rid);
//        cout << "pageNUm:" << rid.getPageNum() << endl << "slot:" << rid.getSlot() << endl;
    }
    rm_manager.CloseFile(rm_fileHandler);

    rm_manager.DestroyFile("test.sql");
}