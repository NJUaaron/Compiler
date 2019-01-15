#ifndef _ICLIST_H_
#define _ICLIST_H_

#include <stdio.h>
#include <stdlib.h>
#include "../../syntax/syntaxTree.h"

struct InterCode{
    enum{ASSIGN, OPERATION, LABEL, _RETURN, GOTO, _RELOP, CALL, READ, WRITE, ARG, PARAM, WRITEMEM, DEC, FUNC} kind;
    struct{
        int vari_no;    //如果是-2表示表达式没有左值，不打印该语句（CALL除外）
    } result;
    struct{
        int type;    //0:variable  1:立即数  2:地址（前面加&） 3:指针（前面加*）
        int val;
    } arg1;
    struct{
        char* type;
    } op;
    struct{
        int type;
        int val;
    } arg2;
    int relop_no; //1~6
    char* func_name;
    struct InterCode* prev;
    struct InterCode* next;
};

extern int IR_no;     //当前可用的最小临时变量号
extern int label_no;     //当前可用的最小label号
extern struct InterCode* ICList;

int new_temp();
int new_label();
struct InterCode* bindCode(struct InterCode* a, struct InterCode* b);

//t:type  v:value
struct InterCode* c_assign(int res, int t1, int v1);
struct InterCode* c_operation(int res, int t1, int v1, char* op, int t2, int v2);
struct InterCode* c_label(int no);
struct InterCode* c_return(int t1, int v1);
struct InterCode* c_goto(int no);
struct InterCode* c_relop(int t1, int v1, int op, int t2, int v2, int label);
struct InterCode* c_call(int return_no, char* func);
struct InterCode* c_read(int return_no);
struct InterCode* c_write(int t1, int v1);
struct InterCode* c_arg(int t1, int v1);
struct InterCode* c_param(int no);
struct InterCode* c_writemem(int no1, int t1, int v1);
struct InterCode* c_decmem(int no, int size);
struct InterCode* c_func(char* func);


//print InterCode
void printInterCode(struct InterCode* head);
void writeInterCode(FILE* f, struct InterCode* head);
#endif