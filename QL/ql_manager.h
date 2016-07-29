//
// Created by 谢俊东 on 16/7/25.
//

#ifndef MINISQLFROM0_QL_MANAGER_H
#define MINISQLFROM0_QL_MANAGER_H

#include <iostream>
#include "../minisql.h"
#include "../RM/rm.h"
#include "../IX/ix.h"
#include "../SM/sm_manager.h"
#include "ql_node.h"

//RC code
#define QL_DUP_ATTR_NAME -1
#define QL_DUP_TABLE_NAME -2
#define QL_ATTR_NOT_EXIST -3
#define QL_TABLE_NOT_EXIST -4
#define QL_INCOMPATIBLE_COMP_OP -5
#define QL_EOF -6

using std::ostream;

struct RelAttr {
    char *relName;     // relation name (may be NULL)
    char *attrName;    // attribute name
    friend ostream &operator<<(ostream &s, const RelAttr &ra);
};

struct Value {
    AttrType type;     // type of value
    void     *data;    // value
    friend ostream &operator<<(ostream &s, const Value &v);
};

struct Condition {
    RelAttr lhsAttr;      // left-hand side attribute
    CompOp  op;           // comparison operator
    int     bRhsIsAttr;   // TRUE if right-hand side is an attribute
    //   and not a value
    RelAttr rhsAttr;      // right-hand side attribute if bRhsIsAttr = TRUE
    Value   rhsValue;     // right-hand side value if bRhsIsAttr = FALSE
    bool (*compartor)(void *value1, void *value2, AttrType attrType, int attrLength);
    friend ostream &operator<<(ostream &s, const Condition &c);
};

//用来记录每一次查询所有属性的信息,多表join的查询时各个表的所有字段都包括进去
struct AttrInfoInRecord {   //字段与AttrCatTuple完全相同,offset字段含义不同
    char relName[MAXNAME];
    char attrName[MAXNAME];
    int offset;             //在整个查询record中的偏移
    AttrType attrType;
    int attrLength;
    int indexNo;
};

class QL_Manager {
    friend class QL_Node;
    friend class QL_RelNode;
    friend class QL_SelNode;
    friend class QL_ProjNode;
    friend class QL_JoinNode;

private:
    SM_Manager &smm;
    IX_Manager &ixm;
    RM_Manager &rmm;

    RelcatTuple *relcatTuples;
    int nRelations;
    AttrInfoInRecord *attrInfoTuples;
    int nAttrInfoInRecord;

    bool HasDupAttrName(int nAttrs, const RelAttr Attrs[]);
    bool HasDupTableName(int nRelations, const char * const relations[]);
    bool CheckTablesValid(int nRelations, const char * const relations[]);
    bool CheckAttrValid(int nAttrs, const RelAttr Attrs[], int nRelations, const char * const relations[]);
    bool CheckCondAttrValid(int nRelations, const char *const relations[], int nCondions, const Condition conditions[]);

    bool CheckCondCompTypeConsistent(int nConditions, const Condition *conditions);
    QL_JoinNode * JoinTwoNode(QL_Node &lRelNode, QL_Node &rRelNode);
public:
    // Constructor
    QL_Manager (SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
    ~QL_Manager ();                         // Destructor
    RC Select (int           nSelAttrs,        // # attrs in Select clause
               const RelAttr selAttrs[],       // attrs in Select clause
               int           nRelations,       // # relations in From clause
               const char * const relations[], // relations in From clause
               int           nConditions,      // # conditions in Where clause
               const Condition conditions[]);  // conditions in Where clause
    RC Insert (const char  *relName,           // relation to insert into
               int         nValues,            // # values to insert
               const Value values[]);          // values to insert
    RC Delete (const char *relName,            // relation to delete from
               int        nConditions,         // # conditions in Where clause
               const Condition conditions[]);  // conditions in Where clause
    RC Update (const char *relName,            // relation to update
               const RelAttr &updAttr,         // attribute to update
               const int bIsValue,             // 0/1 if RHS of = is attribute/value
               const RelAttr &rhsRelAttr,      // attr on RHS of =
               const Value &rhsValue,          // value on RHS of =
               int   nConditions,              // # conditions in Where clause
               const Condition conditions[]);  // conditions in Where clause
};


#endif //MINISQLFROM0_QL_MANAGER_H
