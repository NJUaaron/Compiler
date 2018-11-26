#ifndef _GRAMMARTREE_H_
#define _GRAMMARTREE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Grammar Tree
struct Node{
    int lineno;
    char* name;
    struct Node* child;
    struct Node* sibling;
    int node_type;  //0: terminal  1:nonterminal  2:nonterminal=>empty string
    union{
        char* id_name;    //ID
        int int_val;
        float float_val;
        int relop_no;   //1~6
        struct TypeNode* extra;  //额外字段:用于存储return应该返回的类型
    };
};

struct Node* createNode(char* name, int lineno, int type);
struct Node* createIDNode(int lineno, char* text);
struct Node* createTYPENode(int lineno, char* text);
struct Node* createRELOPNode(int lineno, char* text);
struct Node* createINTNode(char* name, int value, int lineno);
struct Node* createFLOATNode(char* name, float value, int lineno);
void addChild(struct Node* a, struct Node* b);
void addSibling(struct Node* a, struct Node* b);
void print(struct Node* root, int depth);    //print tree

#endif