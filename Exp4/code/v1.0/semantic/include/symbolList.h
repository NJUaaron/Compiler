#ifndef _SIMBOLLIST_H_
#define _SIMBOLLIST_H_

#include <stdio.h>
#include <stdlib.h>
#include "../../syntax/syntaxTree.h"
#include "../../IR/include/ICList.h"
#include "symError.h"
#include "funList.h"

//External Symbol List
//假设：所有变量的作用域都是全局的
struct ExtSListNode{
    char* name;
    int no;    //IR临时变量号
    struct TypeNode* Type;
    struct ExtSListNode* next;
    int inParam;    //是否在函数形参中定义，若是则t_no保存变量的地址（仅针对结构体和数组）
    int inFunc;     //是否在函数内定义
};

extern struct ExtSListNode* ExtSList;

void addExtNode(struct Node* specifier, struct Node* vardec, int pa/*inParam*/, int fu/*inFunc*/);
struct ExtSListNode* InExtList(char* name);  //检查是否在全局符号表中


//TYPE
struct TypeNode{
    char* name;  //没有则为NULL
    int size;  //没有则为-1
    int type;  //int: 0 float: 1 Type: 2
    struct TypeNode* Type;  //when type == 0 or 1, Type == NULL
    struct TypeNode* next;
};

struct TypeNode* getType(struct Node* specifier);
struct TypeNode* getType2(struct Node* specifier, struct Node* vardec);
struct TypeNode* getExpType(struct Node* exp);
int getTypeWidth(struct TypeNode* Type);

int TypeCheck(struct TypeNode* a, struct TypeNode* b);  //检查a和b类型是否相同，相同则返回1
void printSt(struct TypeNode* Type, int depth);  //以二叉树形式打印类型结构


//Struct List
struct StructListNode{
    char* name;
    struct TypeNode* Type;
    struct StructListNode* next;
};

extern struct StructListNode* StructList;

void addStructNode(struct Node* specifier);


#endif