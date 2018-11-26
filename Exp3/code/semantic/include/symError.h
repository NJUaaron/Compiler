#ifndef _SYMERROR_H_
#define _SYMERROR_H_

#include <stdio.h>
#include <stdlib.h>

struct ErrorNode{
    int no;
    int lineno;
    char* symbol;
    struct ErrorNode* next;
};

extern struct ErrorNode* errorList;

void addErrorNode(int no, int lineno, char* symbol);
void errorPrint();
void errorSort();

#endif