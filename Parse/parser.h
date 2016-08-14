//
// parser.h
//   Parser Component Interface
//

#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <cassert>
#include "../minisql.h"
#include "../PF/pf_filemgr.h"

//
// Structure declarations and output functions
//
struct AttrInfo {
    char *attrName;   /* attribute name       */
    AttrType attrType;    /* type of attribute    */
    int attrLength;  /* length of attribute  */
    int bIsPrimary; //是否是主键
};

struct RelAttr {
    RelAttr() {
        relName = NULL;
        attrName = NULL;
    }

    RelAttr(const RelAttr &relAttr) {
        if (relAttr.relName) {
            this->relName = new char[strlen(relAttr.relName) + 1];
            strcpy(this->relName, relAttr.relName);
        } else this->relName = NULL;
        if (relAttr.attrName) {
            this->attrName = new char[strlen(relAttr.attrName) + 1];
            strcpy(this->attrName, relAttr.attrName);
        } else this->attrName = nullptr;
    }

    RelAttr(const char *relName, const char *attrName) {
        if (relName) {
            this->relName = new char[strlen(relName) + 1];
            strcpy(this->relName, relName);
        } else {
            this->relName = NULL;
        }
        this->attrName = new char[strlen(attrName) + 1];
        strcpy(this->attrName, attrName);
    }

    ~RelAttr() {
        if (relName) delete[] relName;
        delete[] attrName;
    }

    RelAttr &operator=(const RelAttr &relAttr) {
        if (this == &relAttr) return *this;
        if (this->relName) delete[] this->relName;
        if (this->attrName) delete[] attrName;
        if (relAttr.relName) {
            this->relName = new char[strlen(relAttr.relName) + 1];
            strcpy(this->relName, relAttr.relName);
        } else this->relName = NULL;
        this->attrName = new char[strlen(relAttr.attrName) + 1];
        strcpy(this->attrName, relAttr.attrName);
        return *this;
    }

    char *relName;    // Relation name (may be NULL)
    char *attrName;   // Attribute name

    // Print function
    friend std::ostream &operator<<(std::ostream &s, const RelAttr &ra);
};

class Value {
public:
    Value() { data = NULL; }

    ~Value() { if (data) delete[] data; }

    Value(const Value &value) {
        type = value.type;
        if(value.data) {
            if(value.type == STRING) {
                data = new char[strlen(value.data) + 1];
                strcpy(data, value.data);
            } else {
                assert(sizeof(int) == sizeof(float));
                data = new char[sizeof(int)];
                memcpy(data, value.data, sizeof(int));
            }
        } else data = nullptr;
    }

    Value &operator=(const Value &value) {
        if (this == &value) return *this;
        type = value.type;
        if (data) delete[] data;
        if(value.data) {
            if(value.type == STRING) {
                data = new char[strlen(value.data) + 1];
                strcpy(data, value.data);
            } else {
                assert(sizeof(int) == sizeof(float));
                data = new char[sizeof(int)];
                memcpy(data, value.data, sizeof(int));
            }
        } else data = nullptr;
        return *this;
    }

    AttrType type;         /* type of value               */
    char *data;        /* value                       */
    /* print function              */
    friend std::ostream &operator<<(std::ostream &s, const Value &v);
};

struct Condition {
    RelAttr lhsAttr;    /* left-hand side attribute            */
    CompOp op;         /* comparison operator                 */
    int bRhsIsAttr; /* TRUE if the rhs is an attribute,    */
    /* in which case rhsAttr below is valid;*/
    /* otherwise, rhsValue below is valid.  */
    RelAttr rhsAttr;    /* right-hand side attribute            */
    Value rhsValue;   /* right-hand side value                */
    /* print function                               */
    friend std::ostream &operator<<(std::ostream &s, const Condition &c);
};

std::ostream &operator<<(std::ostream &s, const CompOp &op);

std::ostream &operator<<(std::ostream &s, const AttrType &at);

//
// Parse function
//
class QL_Manager;

class SM_Manager;

void RBparse(PF_Manager &pfm, SM_Manager &smm, QL_Manager &qlm);

//
// Error printing function; calls component-specific functions
//
void PrintError(RC rc);

// bQueryPlans is allocated by parse.y.  When bQueryPlans is 1 then the
// query plan chosen for the SFW query will be displayed.  When
// bQueryPlans is 0 then no query plan is shown.
//extern int bQueryPlans;

#endif
