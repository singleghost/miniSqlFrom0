//
// Created by 谢俊东 on 16/7/22.
//

#include <zconf.h>
#include <cassert>
#include <sstream>
#include "sm_manager.h"
#include "printer.h"


RC SM_Manager::OpenDb(const char *dbName) {
    chdir(dbName);
    rmm.OpenFile("relcat", relcat_fhandler);
    rmm.OpenFile("attrcat", attrcat_fhandler);
    return 0;
}

RC SM_Manager::CloseDb() {
    rmm.CloseFile(relcat_fhandler);
    rmm.CloseFile(attrcat_fhandler);
    return 0;
}

RC SM_Manager::CreateTable(const char *relName, int attrCount, AttrInfo *attributes) {
    RM_FileScan rm_fileScan;
    RM_Record rec;
    rm_fileScan.OpenScan(relcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *)relName);
    if( rm_fileScan.GetNextRec(rec) != RM_EOF) {
        //重复的表名
        rm_fileScan.CloseScan();
        return SM_DUPLICATE_TABLE;
    }

    char relcatTuple[RELCAT_RECORD_SIZE] = {0}; //relName\tupleLength\attrCount\indexCount
    int tupleLength = 0;
    for (int i = 0; i < attrCount; i++) {
        tupleLength += attributes[i].attrLength;
    }
    createRelCatTuple(relName, tupleLength, attrCount, 0, relcatTuple);
    RID rid;
    relcat_fhandler.InsertRec(relcatTuple, rid);

    char attrcatTuple[ATTRCAT_RECORD_SIZE] = {0};
    int offset = 0;
    for (int i = 0; i < attrCount; i++) {
        createAttrCatTuple(relName, attributes[i].attrName, offset, attributes[i].attrType, attributes[i].attrLength,
                           -1, attrcatTuple);
        offset += attributes[i].attrLength;
        attrcat_fhandler.InsertRec(attrcatTuple, rid);
    }

    //创建数据表文件
    rmm.CreateFile(relName, tupleLength);

    return 0;
}

RC SM_Manager::DropTable(const char *relName) {
    RM_Record rec;
    //删除relcat中的表记录
    rm_fileScan.OpenScan(relcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *) relName);
    if (rm_fileScan.GetNextRec(rec) == RM_EOF) return SM_TABLE_NOT_EXIST;
    int indexCount = ((RelcatTuple *)(rec.GetContent()))->indexCount;
    relcat_fhandler.DeleteRec(rec.GetRid());

    rm_fileScan.CloseScan();

    //删除attrcat中的属性记录
    rm_fileScan.OpenScan(attrcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *) relName);
    while (rm_fileScan.GetNextRec(rec) != RM_EOF) {
        attrcat_fhandler.DeleteRec(rec.GetRid());
    }
    rm_fileScan.CloseScan();

    rmm.DestroyFile(relName);
    for(int i = 0; i < indexCount; i++) {
        stringstream ss;
        ss << i;
        string extend;
        ss >> extend;
        string indexFileName = relName + extend;
        rmm.DestroyFile(indexFileName);
    }
    return 0;

}

RC SM_Manager::CreateIndex(const char *relName, const char *attrName) {
    RM_Record rec;
    rm_fileScan.OpenScan(relcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *) relName);
    if (rm_fileScan.GetNextRec(rec) == RM_EOF) {
        rm_fileScan.CloseScan();
        return SM_TABLE_NOT_EXIST;
    }
    rm_fileScan.CloseScan();


    RM_Record tRec;
    //检查属性是否已经建立索引
    rm_fileScan.OpenScan(attrcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *)relName);
    while(rm_fileScan.GetNextRec(tRec) != RM_EOF) {
        AttrcatTuple *attrcatTuple = (AttrcatTuple *)tRec.GetContent();
        if(strncmp(attrcatTuple->attrName, attrName, MAXNAME) == 0) {
            if(attrcatTuple->indexNo != -1) {
                rm_fileScan.CloseScan();
                return SM_DUPLICATE_INDEX;
            }
        }
    }
    rm_fileScan.CloseScan();

    //relcat中的indexCount加一
    RelcatTuple *relcatTuple = (RelcatTuple *) rec.GetContent();
    int curIndexCnt = relcatTuple->indexCount;
    relcatTuple->indexCount++;
    RID rel_rid = rec.GetRid();
    relcat_fhandler.UpdateRec(RM_Record((char *) relcatTuple, rel_rid, RELCAT_RECORD_SIZE));

    //attrcat中找到对应的属性,改变indexNo
    RM_Record attrRec;
    rm_fileScan.OpenScan(attrcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *) relName);
    while (rm_fileScan.GetNextRec(attrRec) != RM_EOF) {
        if (strncmp(attrRec.GetContent() + MAXNAME, attrName, MAXNAME) == 0) break;
    }
    AttrcatTuple *attrTuple = (AttrcatTuple *) attrRec.GetContent();
    attrTuple->indexNo = curIndexCnt;

    RID rid = attrRec.GetRid();
    attrcat_fhandler.UpdateRec(RM_Record((char *) attrTuple, rid, ATTRCAT_RECORD_SIZE));

    //创建index文件
    ixm.CreateIndex(relName, attrTuple->indexNo, attrTuple->attrType, attrTuple->attrLength);
    return 0;
}

RC SM_Manager::DropIndex(const char *relName, const char *attrName) {
    RM_Record rec;
    rm_fileScan.OpenScan(relcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *) relName);
    if (rm_fileScan.GetNextRec(rec) == RM_EOF) return SM_TABLE_NOT_EXIST;
    rm_fileScan.CloseScan();

    //relcat中的indexCount减一
    RelcatTuple *relcatTuple = (RelcatTuple *) rec.GetContent();
//    int curIndexCnt = relcatTuple->indexCount;
    relcatTuple->indexCount--;
    assert(relcatTuple->indexCount >= 0);
    RID rel_rid = rec.GetRid();
    relcat_fhandler.UpdateRec(RM_Record((char *) relcatTuple, rel_rid, RELCAT_RECORD_SIZE));

    //attrcat中找到对应的属性,改变indexNo
    RM_Record attrRec;
    rm_fileScan.OpenScan(attrcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *) relName);
    while (rm_fileScan.GetNextRec(attrRec) != RM_EOF) {
        if (strncmp(attrRec.GetContent() + MAXNAME, attrName, MAXNAME) == 0) break;
    }
    rm_fileScan.CloseScan();

    AttrcatTuple *attrTuple = (AttrcatTuple *) attrRec.GetContent();
    int indexNo = attrTuple->indexNo;
    attrTuple->indexNo = -1;

    RID rid = attrRec.GetRid();
    attrcat_fhandler.UpdateRec(RM_Record((char *) attrTuple, rid, ATTRCAT_RECORD_SIZE));

    //删除index文件
    ixm.DestroyIndex(relName, indexNo);
    return 0;
}

RC SM_Manager::Help() {
    printf("------------------------\n");
    printf("table list\n");
    printf("------------------------\n");
    RM_Record rec;
    rm_fileScan.OpenScan(relcat_fhandler, STRING, MAXNAME, 0, NO_OP, NULL);
    char relName[MAXNAME + 1];
    while (rm_fileScan.GetNextRec(rec) != RM_EOF) {
        strncpy(relName, rec.GetContent(), MAXNAME);
        printf("%s\n", relName);
    }
    printf("------------------------\n");
    rm_fileScan.CloseScan();
    return 0;
}

RC SM_Manager::Help(const char *relName) {
    RM_Record rec;
    rm_fileScan.OpenScan(relcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *)relName);
    if (rm_fileScan.GetNextRec(rec) == RM_EOF) {
        printf("Table `%s` not exist\n", relName);
        return SM_TABLE_NOT_EXIST;
    }
    AttrcatTuple *attrcatTuple;
    char type[10];
    rm_fileScan.OpenScan(attrcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *) relName);
    printf("-----------------------------------------------------------------\n");
    printf("table structure of `%s`\n", relName);
    printf("-----------------------------------------------------------------\n");
    printf("|attrName\t\t\t\t|type    |length    |offset\t\n");
    printf("-----------------------------------------------------------------\n");
    while (rm_fileScan.GetNextRec(rec) != RM_EOF) {
        attrcatTuple = (AttrcatTuple *) rec.GetContent();
        if (attrcatTuple->attrType == INT) strcpy(type, "int");
        if (attrcatTuple->attrType == FLOAT) strcpy(type, "float");
        if (attrcatTuple->attrType == STRING) strcpy(type, "string");

        printf("|%-23s|%-8s|%-10d|%-10d\n", attrcatTuple->attrName, type, attrcatTuple->attrLength,
               attrcatTuple->offset);
    }
    printf("-----------------------------------------------------------------\n\n");
    rm_fileScan.CloseScan();
    return 0;
}

RC SM_Manager::Print(const char *relName) {
    AttrcatTuple *attrcatTuple;
    RelcatTuple *relcatTuple;

    DataAttrInfo *attributes;
    int attrCount;
    RM_FileHandler rfh;
    RM_Record rec;
    RM_FileScan rfs;
    char *data;

    //先检查表是否存在
    rfs.OpenScan(relcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *)relName);
    if(rfs.GetNextRec(rec) == RM_EOF)
        return SM_TABLE_NOT_EXIST;
    rfs.CloseScan();
    relcatTuple = (RelcatTuple *)rec.GetContent();
    attrCount = relcatTuple->attrCount; //获取表的属性数量

    attributes = new DataAttrInfo[attrCount];
    //填充attributes
    rfs.OpenScan(attrcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *)relName);
    int i = 0;

    while(rfs.GetNextRec(rec) != RM_EOF) {
        attrcatTuple = (AttrcatTuple *)rec.GetContent();
        strncpy(attributes[i].attrName, attrcatTuple->attrName, MAXNAME);
        strncpy(attributes[i].relName, attrcatTuple->relName, MAXNAME);
        attributes[i].offset = attrcatTuple->offset;
        attributes[i].attrLength = attrcatTuple->attrLength;
        attributes[i].attrType = attrcatTuple->attrType;
        attributes[i].indexNo = attrcatTuple->indexNo;
        i++;
    }
    rfs.CloseScan();

// Instantiate a Printer object and print the header information
    Printer p(attributes, attrCount);
    p.PrintHeader(cout);

// Open the file and set up the file scan
    if(relName == "relcat") {
        rfh = relcat_fhandler;
    } else if(relName == "attrcat") {
        rfh = attrcat_fhandler;
    } else {
        rmm.OpenFile(relName, rfh);
    }
    rfs.OpenScan(rfh, INT, sizeof(int), 0, NO_OP, NULL);

// Print each tuple
    while (rfs.GetNextRec(rec) != RM_EOF) {
            data = rec.GetContent();
            p.Print(cout, data);
    }

// Print the footer information
    p.PrintFooter(cout);

// Close the scan, file, delete the attributes pointer, etc.
    rfs.CloseScan();
    if(relName != "relcat" && relName != "attrcat") rmm.CloseFile(rfh);
    delete [] attributes;
    return 0;
}

RC SM_Manager::Load(const char *relName, const char *fileName) {

    AttrcatTuple *attrcatTuple;
    RelcatTuple *relcatTuple;

    DataAttrInfo *attributes;
    int attrCount;
    RM_FileHandler rfh;
    RM_Record rec;
    RM_FileScan rfs;
    char *data;
// Fill in the attributes structure, define the RM_FileHandle
    rfs.OpenScan(relcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *)relName);
    if(rfs.GetNextRec(rec) == RM_EOF)
        return -1;
    relcatTuple = (RelcatTuple *)rec.GetContent();
    attrCount = relcatTuple->attrCount; //获取表的属性数量
    rfs.CloseScan();

    attributes = new DataAttrInfo[attrCount];
    rfs.OpenScan(attrcat_fhandler, STRING, MAXNAME, 0, EQ_OP, (void *)relName);
    int i = 0;
    int tupleLength = 0;
    while(rfs.GetNextRec(rec) != RM_EOF) {
        attrcatTuple = (AttrcatTuple *)rec.GetContent();
        strncpy(attributes[i].attrName, attrcatTuple->attrName, MAXNAME);
        strncpy(attributes[i].relName, attrcatTuple->relName, MAXNAME);
        attributes[i].offset = attrcatTuple->offset;
        attributes[i].attrLength = attrcatTuple->attrLength;
        attributes[i].attrType = attrcatTuple->attrType;
        attributes[i].indexNo = attrcatTuple->indexNo;
        tupleLength += attributes[i].attrLength;
        i++;
    }
    RM_FileHandler rm_fileHandler;
    rmm.OpenFile(relName, rm_fileHandler);
    RID rid;

    FILE *fp = fopen(fileName, "r");
    if(fp == NULL) {
        return SM_DATA_FILE_NOT_EXIST;
    }
    char *line = new char[tupleLength * 5]; //设置一个足够大的能容纳一个tuple的数组
    char *attrEnd;
    char tuple[tupleLength];
    while(!feof(fp)) {
        memset(line, 0, sizeof(line));
        fgets(line, tupleLength * 5, fp);   //读取文件的一行
        if(strcmp(line, "") == 0) break;    //判断是否读取了空行
        char *attrBegin = line;
        memset(tuple, 0, sizeof(tuple));    //清零
        for(i = 0; i < attrCount; i++) {    //循环读取每一个属性
            attrEnd= strchr(attrBegin, ',');
            if (attributes[i].attrType == STRING) {
                strncpy((char *) (tuple + attributes[i].offset), attrBegin, attrEnd - attrBegin);
            }
            else if (attributes[i].attrType == INT) {
                sscanf(attrBegin, "%d", (int *) (tuple + attributes[i].offset));
            }
            else if (attributes[i].attrType == FLOAT) {
                sscanf(attrBegin, "%f", (float *) (tuple + attributes[i].offset));
            }
            if(attrEnd == NULL) break;  //最后一个属性
            attrBegin = attrEnd + 1;
        }
        rm_fileHandler.InsertRec(tuple, rid);
    }
    rmm.CloseFile(rm_fileHandler);
    delete [] line;
    return 0;
}

RC SM_Manager::Set(const char *paramName, const char *value) {
    //TODO
}

void SM_Manager::createRelCatTuple(const char *relName, int tupleLength, int attrCount, int indexCount,
                                   char *relcat_rec) {
    strcpy(relcat_rec, relName);                                   //relName
    *(int *) (&relcat_rec[MAXNAME]) = tupleLength;    //tupleLength
    *(int *) (&relcat_rec[MAXNAME + sizeof(int)]) = attrCount;       //attrCount
    *(int *) (&relcat_rec[MAXNAME + sizeof(int) * 2]) = indexCount;   //number of indexes

}

void SM_Manager::createAttrCatTuple(const char *relName, const char *attrName, int offset, AttrType attrType,
                                    int attrLength, int indexNo, char *attrcat_rec) {
    strcpy(attrcat_rec, relName);
    strcpy(attrcat_rec + MAXNAME, attrName);
    *(int *) (&attrcat_rec[MAXNAME * 2]) = offset;
    *(AttrType *) (&attrcat_rec[MAXNAME * 2 + sizeof(int)]) = attrType;
    *(int *) (&attrcat_rec[MAXNAME * 2 + sizeof(int) + sizeof(AttrType)]) = attrLength;
    *(int *) (&attrcat_rec[MAXNAME * 2 + sizeof(int) + sizeof(AttrType) + sizeof(int)]) = indexNo;

}
