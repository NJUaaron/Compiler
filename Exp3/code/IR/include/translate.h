#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

//translate diagnose mode
//#define T_DIAG_MODE

#include <stdio.h>
#include <stdlib.h>
#include "ICList.h"
#include "../../semantic/include/funList.h"

//形参列表节点
struct argList{
    int type;
    int val;
    struct argList* next;
};

struct InterCode* translate_Exp(struct Node* exp, int place); //目前还不能将函数和结构体作为形参
struct InterCode* translate_Args(struct Node* args, struct argList* arg_list);
struct InterCode* translate_addr(struct Node* exp, int place);   //place代表的变量记录exp的地址
struct InterCode* translate_Stmt(struct Node* stmt);
struct InterCode* translate_Cond(struct Node* exp, int label_true, int label_false);

struct InterCode* translate_Program(struct Node* program);
struct InterCode* translate_ExtDefList(struct Node* extdeflist);
struct InterCode* translate_ExtDef(struct Node* extdef);
struct InterCode* translate_ExtDecList(struct Node* extdeclist);
struct InterCode* translate_VarList(struct Node* varlist);
struct InterCode* translate_CompSt(struct Node* compst);
struct InterCode* translate_DefList(struct Node* deflist);
struct InterCode* translate_StmtList(struct Node* stmtlist);
struct InterCode* translate_Def(struct Node* def);

#endif