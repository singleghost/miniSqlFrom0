#include "bm_hashTable.h"
#include "bm_error.h"

BM_HashTable::BM_HashTable(int numOfBuckets) : numOfBuckets(numOfBuckets) {
    HashTable = (TableEntry **)calloc(this->numOfBuckets, sizeof(TableEntry *));
}


BM_HashTable::~BM_HashTable() {
    for(auto i = 0; i < numOfBuckets; i++) {
        TableEntry *head = HashTable[i];
        if(HashTable[i]) head = HashTable[i]->next;
        TableEntry *temp = head;
        while (head != NULL) {
            temp = head->next;
            free(head);
            head = temp;
        }
    }

    free(this->HashTable);
}

int BM_HashTable::Hash(int fd, int pageNum) {
    return ((fd << 4) + pageNum) % numOfBuckets;
}

//链表的表头插入
RC BM_HashTable::Insert(int fd, int pageNum, int slot) {
    TableEntry *newEntry = new TableEntry;
    newEntry->fd = fd;
    newEntry->pageNum = pageNum;
    newEntry->slot = slot;

    int index = Hash(fd, pageNum);
    newEntry->next = HashTable[index];

    HashTable[index] = newEntry;
    return 0;
}

RC BM_HashTable::Find(int fd, int pageNum, int &slot) {
    int index = Hash(fd, pageNum);
    TableEntry *entry = HashTable[index];
    while (entry != NULL) {
        if(entry->fd == fd && entry->pageNum == pageNum) {
            slot = entry->slot;
            return 0;
        }
        entry = entry->next;
    }
    slot = INVALID_SLOT;
    return 0;
}

RC BM_HashTable::Delete(int fd, int pageNum) {
    int index = Hash(fd, pageNum);
    TableEntry *entry = HashTable[index];
    TableEntry *temp;
    if(entry->fd == fd && entry->pageNum == pageNum) {
        HashTable[index] = entry->next;
        free(entry);
        return 0;
    }
    while (entry != NULL) {
        temp = entry;
        entry = entry->next;
        if(fd == entry->fd && pageNum == entry->pageNum) {
            temp->next = entry->next;
            free(entry);
            return 0;
        }
    }
    return SLOT_NOT_FOUND_WARNING;
}