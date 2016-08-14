//
// Created by 谢俊东 on 16/7/22.
//

#include <iostream>
#include <zconf.h>
#include "PF/pf_filemgr.h"
#include "RM/rm.h"
#include "SM/sm_manager.h"

using namespace std;


int main ( int argc, char **argv )
{
    char *dbname;
    char command[80] = "mkdir ";

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " dbname \n";
        exit(1);
    }

    // The database name is the second argument
    dbname = argv[1];

    // Create a subdirectory for the database
    system (strncat(command,dbname, 70));

    if (chdir(dbname) < 0) {
        cerr << argv[0] << " chdir error to " << dbname << "\n";
        exit(1);
    }

    PF_Manager pfm;
    RM_Manager rmm(pfm);

    //创建两个system catalog文件
    rmm.CreateFile("relcat", RELCAT_RECORD_SIZE);
    rmm.CreateFile("attrcat", ATTRCAT_RECORD_SIZE);

    RM_FileHandler relcat_fh;
    RM_FileHandler attrcat_fh;
    rmm.OpenFile("relcat", relcat_fh);
    rmm.OpenFile("attrcat", attrcat_fh);

    //relation catalog 和attribute catalog需要有关于自身信息的tuple
    char relcat_rec[RELCAT_RECORD_SIZE] = {0};
    SM_Manager::createRelCatTuple("relcat", RELCAT_RECORD_SIZE, 4, 0, relcat_rec);

    RID rid;
    relcat_fh.InsertRec(relcat_rec, rid);

    SM_Manager::createRelCatTuple("attrcat", ATTRCAT_RECORD_SIZE, 7, 0, relcat_rec);
    relcat_fh.InsertRec(relcat_rec, rid);

    char attrcat_rec[ATTRCAT_RECORD_SIZE] = {0};
    SM_Manager::createAttrCatTuple("relcat", "relName", 0, STRING, MAXNAME, -1, 1, attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);
    SM_Manager::createAttrCatTuple("relcat", "tupleLength", MAXNAME, INT, 4, -1, 0, attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);
    SM_Manager::createAttrCatTuple("relcat", "attrCount", MAXNAME + sizeof(int), INT, 4, -1, 0, attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);
    SM_Manager::createAttrCatTuple("relcat", "indexCount", MAXNAME + sizeof(int) * 2, INT, 4, -1, 0, attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);

    SM_Manager::createAttrCatTuple("attrcat", "relName", 0, STRING, MAXNAME, -1, 0, attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);
    SM_Manager::createAttrCatTuple("attrcat", "attrName", MAXNAME, STRING, MAXNAME, -1, 0, attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);
    SM_Manager::createAttrCatTuple("attrcat", "offset", MAXNAME * 2, INT, 4, -1, 0, attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);
    SM_Manager::createAttrCatTuple("attrcat", "attrType", MAXNAME * 2 + sizeof(int), INT, sizeof(int), -1, 0,
                                   attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);
    SM_Manager::createAttrCatTuple("attrcat", "attrLength", MAXNAME * 2 + sizeof(int) * 2, INT, sizeof(int), -1, 0,
                                   attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);
    SM_Manager::createAttrCatTuple("attrcat", "indexNo", MAXNAME * 2 + sizeof(int) * 3, INT, sizeof(int), -1, 0,
                                   attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);
    SM_Manager::createAttrCatTuple("attrcat", "isPrimary", MAXNAME * 2 + sizeof(int) * 4, INT, sizeof(int), -1, 0, attrcat_rec);
    attrcat_fh.InsertRec(attrcat_rec, rid);

    rmm.CloseFile(relcat_fh);
    rmm.CloseFile(attrcat_fh);
}