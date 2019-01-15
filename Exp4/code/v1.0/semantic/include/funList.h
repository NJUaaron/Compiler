#ifndef _FUNLIST_H_
#define _FUNLIST_H_

#include <stdio.h>
#include <stdlib.h>
#include "symbolList.h"

//Function List
struct paraNode{
    char* name;
    struct TypeNode* Type;
    struct paraNode* next;
};

struct FunListNode{
    int funtype;  //声明:0 定义:1
    int lineno;
    char* name;  
    struct TypeNode* returnType;  //返回值类型
    struct paraNode* para;  //形参列表
    struct FunListNode* next;
};

extern struct FunListNode* FunList;

void addFunNode(struct Node* extdef);

//print function list
void printFunList();  

//检查两个函数返回类型、形参数量和类型是否一致，一致则返回1
int checkFun(struct FunListNode* a, struct FunListNode* b);

//检查函数参数是否符合定义, 符合则返回1
int checkPara(struct FunListNode* def, struct Node* args);

//检查函数体中的return是否正确
void checkReturn(struct Node* stmtlist, struct TypeNode* returnType);

//检查是否有未定义的函数声明，有则返回1
int checkNoDef();

//填充函数的形参列表
struct paraNode* FillparaList(struct Node* varlist);

//向函数表中预先加载read和write函数
void preload();


#endif