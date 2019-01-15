#include "syntaxTree.h"

struct Node* createNode(char* name, int lineno, int type){
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->name = name;
    node->lineno = lineno;
    node->node_type = type;
    node->child = NULL;
    node->sibling = NULL;
    return node;
}

struct Node* createIDNode(int lineno, char* text){
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->name = "ID";
    node->lineno = lineno;
    node->node_type = 0;
    char* buf = (char*)malloc(33);
    strcpy(buf, text);
    node->id_name = buf;
    node->child = NULL;
    node->sibling = NULL;
    return node;
}

struct Node* createTYPENode(int lineno, char* text){
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->name = "TYPE";
    node->lineno = lineno;
    node->node_type = 0;
    char* buf = (char*)malloc(33);
    strcpy(buf, text);
    node->id_name = buf;
    node->child = NULL;
    node->sibling = NULL;
    return node;
}

struct Node* createRELOPNode(int lineno, char* text){
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->name = "RELOP";
    node->lineno = lineno;
    node->node_type = 0;
    node->child = NULL;
    node->sibling = NULL;
    if (!strcmp(text, ">"))
        node->relop_no = 1;
    else if (!strcmp(text, "<"))
        node->relop_no = 2;
    else if (!strcmp(text, ">="))
        node->relop_no = 3;
    else if (!strcmp(text, "<="))
        node->relop_no = 4;
    else if (!strcmp(text, "=="))
        node->relop_no = 5;
    else if (!strcmp(text, "!="))
        node->relop_no = 6;
    return node;
}

struct Node* createINTNode(char* name, int value, int lineno){
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->name = name;
    node->lineno = lineno;
    node->int_val = value;
    node->node_type = 0;
    node->child = NULL;
    node->sibling = NULL;
    return node;
}

struct Node* createFLOATNode(char* name, float value, int lineno){
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->name = name;
    node->lineno = lineno;
    node->float_val = value;
    node->node_type = 0;
    node->child = NULL;
    node->sibling = NULL;
    return node;
}

void addChild(struct Node* a, struct Node* b){
    a->child = b;

    //非终结节点的行号等于第一个子节点的行号
    if (a->node_type == 1)    
        a->lineno = b->lineno;
}

void addSibling(struct Node* a, struct Node* b){
    a->sibling = b;
}

void print(struct Node* root, int depth){
    if (root == NULL)
        return;
    
    //推导出空串的非终结符号不进行打印
    if (root->node_type != 2){
        for(int i = 0; i < depth; i++)    //retract
            printf("  ");
        printf("%s",root->name);

        if (!strcmp(root->name, "ID")||!strcmp(root->name, "TYPE"))
            printf(" : %s ",root->id_name);
        else if (!strcmp(root->name, "INT"))
            printf(" : %d", root->int_val);
        else if (!strcmp(root->name, "FLOAT"))
            printf(" : %f", root->float_val);
        else if (root->node_type == 1)    //非终结符号打印行号
            printf(" (%d)", root->lineno);
        printf("\n");
    }

    print(root->child, depth + 1);    //此处不能使用++depth或depth++
    print(root->sibling, depth);
}
