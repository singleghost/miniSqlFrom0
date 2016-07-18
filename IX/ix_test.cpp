//
// Created by 谢俊东 on 16/7/17.
//

#include "ix.h"

int main()
{
    PF_Manager pfm;
    IX_Manager ixm(pfm);
    IX_IndexHandler indexHandler;
    ixm.CreateIndex("zju", 0, INT, 4);
    ixm.OpenIndex("zju", 0, indexHandler);

    int num;
    int i, j;
    i = 0;
    j = 0;
    for(num = 0; num < 400; num++) {
        RID rid(i, j);
        indexHandler.InsertEntry(&num, rid);
        j++;
        if(j >= 100) {
            i++;
            j = 0;
        }
    }

    ixm.CloseIndex(indexHandler);
    ixm.DestroyIndex("zju", 0);
}

