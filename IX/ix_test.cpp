//
// Created by 谢俊东 on 16/7/17.
//

#include "ix.h"

#define INSERT_NUM 500
int main()
{
    PF_Manager pfm;
    IX_Manager ixm(pfm);
    IX_IndexHandler indexHandler;
    IX_IndexScan indexScan;
    ixm.CreateIndex("zju", 0, INT, 4);
    ixm.OpenIndex("zju", 0, indexHandler);

    char value[100];
    int k;
    int i, j;
    i = 0;
    j = 0;
    for(k= 0; k < INSERT_NUM; k++){   //parentNode似乎还有点问题
        RID rid(i, j);
//        memset(value, 0, 100);
//        sprintf(value, "%d", k);
        indexHandler.InsertEntry(&k, rid);
        j++;
        if(j >= 100) {
            i++;
            j = 0;
        }
    }
    indexHandler.PrintAll();
    printf("\n-----------insert over------------\n\n");

    int cmpValue = 100;
    indexScan.OpenScan(indexHandler, LE_OP, &cmpValue);
    RID res_rid(-1, -1);
    while(true) {

        if( indexScan.GetNextEntry(res_rid) == IX_NO_MORE_ENTRY) break;
        cout << "pageNum:" << res_rid.getPageNum() << "\t" << "slot:" << res_rid.getSlot() << endl;
    }

    i = 0;
    j = 0;
//
    for(k= 0; k <INSERT_NUM; k++){
//        memset(value, 0, 100);
//        sprintf(value, "%d", k);
//        if(k ==4725) pfm.GetBufferManager()->PrintPageDescTable();
        RID rid(i, j);
        indexHandler.DeleteEntry(&k, rid);
        j++;
        if(j >= 100) {
            i++;
            j = 0;
        }
    }
    indexHandler.PrintAll();
    indexScan.CloseScan();
    ixm.CloseIndex(indexHandler);


    //第二轮
    printf("---------------second round\n\n--------------");
    ixm.OpenIndex("zju", 0, indexHandler);

    i = 0;
    j = 0;
    for(k= 0; k < INSERT_NUM; k++){   //parentNode似乎还有点问题
        RID rid(i, j);
//        memset(value, 0, 100);
//        sprintf(value, "%d", k);
        indexHandler.InsertEntry(&k, rid);
        j++;
        if(j >= 100) {
            i++;
            j = 0;
        }
    }
    indexHandler.PrintAll();
    printf("\n-----------insert over------------\n\n");

    cmpValue = 100;
    indexScan.OpenScan(indexHandler, LE_OP, &cmpValue);
    res_rid = RID(-1, -1);
    while(true) {

        if( indexScan.GetNextEntry(res_rid) == IX_NO_MORE_ENTRY) break;
        cout << "pageNum:" << res_rid.getPageNum() << "\t" << "slot:" << res_rid.getSlot() << endl;
    }

    i = 0;
    j = 0;
//
    for(k= 0; k <INSERT_NUM; k++){
//        memset(value, 0, 100);
//        sprintf(value, "%d", k);
//        if(k ==4725) pfm.GetBufferManager()->PrintPageDescTable();
        RID rid(i, j);
        indexHandler.DeleteEntry(&k, rid);
        j++;
        if(j >= 100) {
            i++;
            j = 0;
        }
    }
    indexHandler.PrintAll();
    indexScan.CloseScan();
    ixm.DestroyIndex("zju", 0);
}

