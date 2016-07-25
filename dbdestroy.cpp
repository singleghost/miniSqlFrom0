//
// Created by 谢俊东 on 16/7/22.
//

#include <iostream>
#include <zconf.h>

using namespace std;

int main ( int argc, char **argv )
{
    char *dbname;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " dbname \n";
        exit(1);
    }

    // The database name is the second argument
    dbname = argv[1];

    char command[80] = "rm -r ";
    system(strncat(command, dbname, 70));
}