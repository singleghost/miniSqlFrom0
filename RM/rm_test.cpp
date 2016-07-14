#include <iostream>
#include "rm.h"

#define RECORD_SIZE 30
using namespace std;
int main()
{
    cout << "start testing\n";

    PF_Manager pfm;
    RM_Manager rm_manager(pfm);

    rm_manager.CreateFile("test.sql", RECORD_SIZE);

    RM_FileHandler rm_fileHandler;
    rm_manager.OpenFile("test.sql", rm_fileHandler);
    rm_manager.CloseFile(rm_fileHandler);
    rm_manager.DestroyFile("test.sql");
}