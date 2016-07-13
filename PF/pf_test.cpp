//
// Created by 谢俊东 on 16/7/12.
//
#include <iostream>
#include "pf_filemgr.h"

using namespace std;
int main()
{
    PF_Manager pf_manager;
    pf_manager.CreateFile("test.sql");
    FileHandler fhandler;
    pf_manager.OpenFile("test.sql", fhandler);
    PageHandler phandler;

    for(int i = 0; i < 200; i++) {
        fhandler.AllocatePage(phandler);
        sprintf(phandler.GetDataPtr(), "this is page %d\n", i);
        fhandler.UnpinPage(phandler.GetPageNum());
    }


    for(int i = 0; i < 50; i++) {
        fhandler.DisposePage(i);
    }
    pf_manager.CloseFile(fhandler);

    pf_manager.OpenFile("test.sql", fhandler);


    FileHandler fileHdlr;
    fileHdlr = fhandler;
    PageHandler phand(phandler);
    try {
        fileHdlr.GetLastPage(phand);
        cout << "page num:" << phand.GetPageNum() << endl;
        cout << "page content:" << phand.GetDataPtr() << endl;
        fileHdlr.UnpinPage(phand.GetPageNum());
    } catch (page_not_found_exception) {

    }
    bool hasNext = true;
    while (hasNext) {
        try {
            fileHdlr.GetPrevPage(phand.GetPageNum(), phand);
            cout << "page num:" << phand.GetPageNum() << endl;
            cout << "page content:" << phand.GetDataPtr() << endl;
            fileHdlr.UnpinPage(phand.GetPageNum());

        } catch (page_not_found_exception) {
            hasNext = false;
        }
    }
    pf_manager.DestroyFile("test.sql");
}