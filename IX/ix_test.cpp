//
// Created by 谢俊东 on 16/7/17.
//

#include "ix.h"

int main()
{
    PF_Manager pfm;
    IX_Manager ixm(pfm);
    IX_IndexHandler indexHandler;
    IX_IndexScan indexScan;
    ixm.CreateIndex("zju", 0, INT, 4);
    ixm.OpenIndex("zju", 0, indexHandler);

    int num;
    int i, j;
    i = 0;
    j = 0;
    for(num = 0; num < 400; num++) {
        RID rid(i, j);
        if(num == 50) {
//            indexHandler.PrintAll();
        }
        indexHandler.InsertEntry(&num, rid);
        j++;
        if(j >= 100) {
            i++;
            j = 0;
        }
    }
//    indexHandler.PrintAll();

    int cmpValue = 400;
    indexScan.OpenScan(indexHandler, EQ_OP, &cmpValue);
    RID res_rid(-1, -1);
    while(true) {

        if( indexScan.GetNextEntry(res_rid) == IX_NO_MORE_ENTRY) break;
        cout << "pageNum:" << res_rid.getPageNum() << endl << "slot:" << res_rid.getSlot() << endl;
    }


    i = 0;
    j = 0;
    for(num = 0; num < 100; num++) {
        RID rid(i, j);
        indexHandler.DeleteEntry(&num, rid);
        j++;
        if(j >= 100) {
            i++;
            j = 0;
        }
    }

    indexHandler.PrintAll();
    ixm.CloseIndex(indexHandler);
    ixm.DestroyIndex("zju", 0);
}

