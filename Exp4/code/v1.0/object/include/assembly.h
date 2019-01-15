#ifndef _ASSEMBLY_H_
#define _ASSEMBLY_H_

#include <stdio.h>
#include <stdlib.h>
#include "../../IR/include/ICList.h"
#include "reg.h"

//汇编部分诊断模式
#define A_DIAG_MODE

//在栈中为所有的临时变量也分配空间
#define SAVE_ALL


struct Assembly{
    char op[5];

    int arg_num;    //参数个数
    struct
    {
        int type;    //0:register  1:立即数  2:label  3:变量名(用于访问静态数据区)
        int val;
    } arg[3];
    char func_name[20];

    struct Assembly* prev;
    struct Assembly* next;
};

struct Assembly *translate_IC(struct InterCode *head);

struct Assembly *create_as(char* op, int arg_num, int t1, int v1, int t2, int v2, int t3, int v3, char* func_name);   //Assembly的构造函数
struct Assembly *bind(struct Assembly *a, struct Assembly *b);

struct Assembly *translate_ICunit(struct InterCode *node);

void writeAssembly(FILE *f, struct Assembly *head); //输出汇编代码到文件

#endif