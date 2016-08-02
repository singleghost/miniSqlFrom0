//
// Created by 谢俊东 on 16/7/10.
//


#include <fcntl.h>
#include <zconf.h>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include "bm_buffermgr.h"

using namespace std;

int main() {
    cout << "start testing" << endl;
    char content[PAGE_SIZE];
    sprintf(content, "This is a test file, can you see me?\n");

    BM_BufferMgr bufferMgr(100);
    FILE *fp = fopen("test.txt", "w");
    fwrite(content, 1, PAGE_SIZE, fp);

    fclose(fp);

    int fd = open("test.txt", O_RDWR);

    char *p;
    for (int i = 0; i < 80; i++) {
        if (bufferMgr.AllocatePage(fd, i, p)) {
            printf("allocate page failure\n");
            exit(-1);
        }
        sprintf(p, "this is page %d\n", i);
        bufferMgr.MarkDirty(fd, i);
        bufferMgr.UnPinPage(fd, i);
    }
    bufferMgr.PrintPageDescTable();
    bufferMgr.writeBackAllDirty(fd);

    char dest[PAGE_SIZE];
    for (int i = 0; i < 80; i++) {
        bufferMgr.ReadPage(fd, i, dest);
        printf("%s", dest);
    }

    for (int i = 0; i < 50; i++) {
        bufferMgr.GetPage(fd, i, p);
        printf("%s", p);
        bufferMgr.UnPinPage(fd, i);
    }

    bufferMgr.PrintPageDescTable();
    for (int i = 80; i < 120; i++) {
        if (bufferMgr.AllocatePage(fd, i, p)) {
            printf("AllocatePage failure\n");
            exit(1);
        }
    }
    printf("after allocate 80~120\n");
    bufferMgr.PrintPageDescTable();


}
