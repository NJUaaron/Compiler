#include "symError.h"

struct ErrorNode* errorList = NULL;

void addErrorNode(int no, int lineno, char* symbol){
    struct ErrorNode* node = (struct ErrorNode*)malloc(sizeof(struct ErrorNode));
    node->no = no;
    node->lineno = lineno;
    node->symbol = symbol;
    node->next = errorList;
    errorList = node;
}

void errorPrint(){
    struct ErrorNode* e = errorList;
    while (e != NULL){
        switch (e->no){
            case 1: printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", e->lineno, e->symbol); break;
            case 2: printf("Error type 2 at Line %d: Undefined function \"%s\".\n", e->lineno, e->symbol); break;
            case 3: printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", e->lineno, e->symbol); break;
            case 4: printf("Error type 4 at Line %d: Redefined function \"%s\".\n", e->lineno, e->symbol); break;
            case 5: printf("Error type 5 at Line %d: Type mismatched for assignment.\n", e->lineno); break;
            case 6: printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", e->lineno); break;
            case 7: printf("Error type 7 at Line %d: Type mismatched for operands.\n", e->lineno); break;
            case 8: printf("Error type 8 at Line %d: Type mismatched for return.\n", e->lineno); break;
            case 9: printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n", e->lineno, e->symbol); break;//要求的简化版本
            case 10: printf("Error type 10 at Line %d: \"%s\" is not an array.\n", e->lineno, e->symbol); break;
            case 11: printf("Error type 11 at Line %d: \"%s\" is not a function.\n", e->lineno, e->symbol); break;
            case 12: printf("Error type 12 at Line %d: array index should be an integer.\n", e->lineno); break;
            case 13: printf("Error type 13 at Line %d: Illegal use of \".\".\n", e->lineno); break;
            case 14: printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", e->lineno, e->symbol); break;
            case 15: printf("Error type 15 at Line %d: Redefined field \"%s\"\n", e->lineno, e->symbol); break;
            case 16: printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", e->lineno, e->symbol); break;
            case 17: printf("Error type 17 at Line %d: Undefined struct \"%s\".\n", e->lineno, e->symbol); break;
            case 18: printf("Error type 18 at Line %d: Undefined function \"%s\".\n", e->lineno, e->symbol); break;
            case 19: printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n", e->lineno, e->symbol); break;
            default: printf("Unknown error type\n");
        }
        e = e->next;
    }
}

void errorSort(){
    if (errorList == NULL||errorList->next == NULL)
        return;

    //创建链表头
    struct ErrorNode* head = (struct ErrorNode*)malloc(sizeof(struct ErrorNode));
    head->next = errorList;

    struct ErrorNode *pre, *cur, *next, *end, *temp;
    end = NULL;
    //从链表头开始将较大值往后沉(bubble sort)
    while(head->next != end)
    {
        for(pre = head, cur = pre->next, next = cur->next; next != end; pre = pre->next, cur = cur->next, next = next->next)
        {
            //相邻的节点比较
            if(cur->lineno > next->lineno)
            {
                cur->next = next->next;
                pre->next = next;
                next->next = cur;
                temp = next;
                next = cur;
                cur = temp;
            }
        }
        end = cur;
    }

    errorList = head->next;
}