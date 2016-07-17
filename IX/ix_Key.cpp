//
// Created by 谢俊东 on 16/7/17.
//
#include "ix.h"
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
