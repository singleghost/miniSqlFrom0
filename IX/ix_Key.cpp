//
// Created by 谢俊东 on 16/7/17.
//
#include "ix.h"

Key::Key(const Key &key) {
    this->attrType = key.attrType;
    this->attrLength = key.attrLength;
    this->ptr = new char[attrLength];
}

Key& Key::operator=(const Key &key) {
    if(this == &key) {
        return *this;
    } else {
        if(this->ptr) delete [] this->ptr;
        this->attrLength = key.attrLength;
        this->attrType = key.attrType;
        this->ptr = new char[attrLength];
        memcpy(this->ptr, key.ptr, attrLength);
        return *this;
    }

}
bool Key::operator==(const Key &key1) const
{
        switch (attrType) {
            case INT: return *(int *)this->ptr == *(int *)key1.ptr;
            case FLOAT: return *(float *)this->ptr == *(float *)key1.ptr;
            case STRING: return strncmp(this->ptr, key1.ptr, attrLength) == 0;
        }
}

bool Key::operator<(const Key &key1) const
{
    switch (attrType) {
        case INT: return *(int *)this->ptr < *(int *)key1.ptr;
        case FLOAT: return *(float *)this->ptr < *(float *)key1.ptr;
        case STRING: return strncmp(this->ptr, key1.ptr, attrLength) < 0;
    }
}

bool Key::operator>(const Key &key1) const
{
    switch (attrType) {
        case INT: return *(int *)this->ptr > *(int *)key1.ptr;
        case FLOAT: return *(float *)this->ptr > *(float *)key1.ptr;
        case STRING: return strncmp(this->ptr, key1.ptr, attrLength) > 0;
    }
}
bool Key::operator<=(const Key &key1) const
{
    switch (attrType) {
        case INT: return *(int *)this->ptr <= *(int *)key1.ptr;
        case FLOAT: return *(float *)this->ptr <= *(float *)key1.ptr;
        case STRING: return strncmp(this->ptr, key1.ptr, attrLength) <= 0;
    }
}
bool Key::operator>=(const Key &key1) const
{
    switch (attrType) {
        case INT: return *(int *)this->ptr >= *(int *)key1.ptr;
        case FLOAT: return *(float *)this->ptr >= *(float *)key1.ptr;
        case STRING: return strncmp(this->ptr, key1.ptr, attrLength) >= 0;
    }
}
bool Key::operator!=(const Key &key1) const
{
    switch (attrType) {
        case INT: return *(int *)this->ptr != *(int *)key1.ptr;
        case FLOAT: return *(float *)this->ptr != *(float *)key1.ptr;
        case STRING: return strncmp(this->ptr, key1.ptr, attrLength) != 0;
    }
}
