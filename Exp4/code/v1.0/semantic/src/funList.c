#include "funList.h"

struct FunListNode* FunList = NULL;

void addFunNode(struct Node* extdef){
    struct Node* fundec = extdef->child->sibling;
    struct FunListNode* node = (struct FunListNode*)malloc(sizeof(struct FunListNode));
    
    //get line no
    node->lineno = extdef->lineno;

    //get funtype
    if (!strcmp(extdef->child->sibling->sibling->name, "SEMI"))
        node->funtype = 0;
    else
        node->funtype = 1;

    //get function name
    char* buf = (char*)malloc(33);
    strcpy(buf, fundec->child->id_name);
    node->name = buf;
    
    //get parameter list
    if (!strcmp(fundec->child->sibling->sibling->name, "VarList"))
        node->para = FillparaList(fundec->child->sibling->sibling);
    else
        node->para = NULL;
        
    //get type of return parameter
    struct Node* specifier = extdef->child;
    node->returnType = getType(specifier);

    //check
    struct FunListNode* ptr = FunList;
    while (ptr != NULL){
        if (!strcmp(ptr->name, node->name)){
            //check repetition in name
            if (node->funtype == 1&&ptr->funtype == 1){
                addErrorNode(4, extdef->lineno, node->name);
                break;
            }

            //check conflict in declaration
            else{
                if (checkFun(node, ptr) == 0){
                    addErrorNode(19, extdef->lineno, node->name);
                    break;
                }
            }

        }
        ptr = ptr->next;
    }

    //add to the function list
    node->next = FunList;
    FunList = node;
}

void printFunList(){
    struct FunListNode* ptr = FunList;
    while(ptr != NULL){
        printf("type: %d name: %s, return:\n", ptr->funtype, ptr->name);
        printSt(ptr->returnType, 0);
        printf("\n    para: ");
        struct paraNode* ptr2 = ptr->para;
        while(ptr2 != NULL){
            printf("%s ", ptr2->name);
            ptr2 = ptr2->next;
        }
        printf("\n\n");
        ptr = ptr->next;
    }
}

int checkFun(struct FunListNode* a, struct FunListNode* b){
    if (TypeCheck(a->returnType, b->returnType) == 0)
        return 0;
    struct paraNode* p = a->para;
    struct paraNode* q = b->para;
    while(1){
        if (p == NULL&&q == NULL)
            return 1;
        if (p == NULL)
            return 0;
        if (q == NULL)
            return 0;
        if (TypeCheck(p->Type, q->Type) == 0)
            return 0;
        p = p->next;
        q = q->next;
    }
}

int checkPara(struct FunListNode* def, struct Node* args){
    struct paraNode* pa = def->para;
    struct Node* exp = args->child;
    if (pa == NULL&&exp == NULL)
        return 1;
    if (pa == NULL)
        return 0;
    if (exp == NULL)
        return 0;

    while (1){
        struct TypeNode* p = pa->Type;
        struct TypeNode* q = getExpType(exp);  //get exp's Type

        if (TypeCheck(p, q) == 0)
            return 0;
        if (pa->next == NULL&&exp->sibling == NULL)
            return 1;
        if (pa->next == NULL)
            return 0;
        if (exp->sibling == NULL)
            return 0;

        pa = pa->next;
        exp = exp->sibling->sibling->child;
    }
}

void checkReturn(struct Node* stmtlist, struct TypeNode* returnType){
    //check all the return in stmtlist
    if (stmtlist == NULL)
        return;
    if (!strcmp(stmtlist->name, "RETURN"))
        stmtlist->extra = returnType;

    checkReturn(stmtlist->child, returnType);
    checkReturn(stmtlist->sibling, returnType);
}

int checkNoDef(){
    int existNoDef = 0;
    struct FunListNode* p = FunList;
    while(p != NULL){
        if (p->funtype == 0){//declaration
            int flag = 0;
            struct FunListNode* q = FunList;
            while(q != NULL){
                if (q->funtype == 1){//definition
                    if (!strcmp(p->name, q->name)&&checkFun(p, q)){//match
                        flag = 1;
                        break;
                    }
                }
                q = q->next;
            }
            if (flag == 0){
                existNoDef = 1;
                addErrorNode(18, p->lineno, p->name);
            }
        }

        p = p->next;
    }
    return existNoDef;
}

struct paraNode* FillparaList(struct Node* varlist){
    struct paraNode* para = NULL;
    struct paraNode* tail = NULL;

    int repe = 1;  //循环
    while (repe){

        struct paraNode* node = (struct paraNode*)malloc(sizeof(struct paraNode));

        //get type
        struct Node* specifier = varlist->child->child;
        struct Node* vardec = specifier->sibling;                
        node->Type = getType2(specifier, vardec);

        //get name
        struct Node* nont = vardec->child;
        while (strcmp(nont->name, "ID"))
            nont = nont->child;
        char* buf = (char*)malloc(33);
        strcpy(buf, nont->id_name);
        node->name = buf;

        //add to the formal parameter list
        node->next = NULL;
        if (tail == NULL){
            para = node;
            tail = node;
        }
        else{
            tail->next = node;
            tail = node;
        }

        //check: VarList -> ParamDec COMMA VarList
        if (varlist->child->sibling != NULL)
            varlist = varlist->child->sibling->sibling;
        else
            repe = 0;
    }

    return para;
}

void preload(){

    struct TypeNode* int_type = (struct TypeNode*)malloc(sizeof(struct TypeNode));
    int_type->name = NULL;
    int_type->size = -1;
    int_type->type = 0;
    int_type->Type = NULL;
    int_type->next = NULL;

    //int read()
    struct FunListNode* read = (struct FunListNode*)malloc(sizeof(struct FunListNode));
    read->funtype = 1;
    read->name = (char*)malloc(33);
    read->name = "read";
    read->returnType = int_type;
    read->para = NULL;
    read->next = FunList;
    FunList = read;

    //int write(int)
    struct FunListNode* write = (struct FunListNode*)malloc(sizeof(struct FunListNode));
    write->funtype = 1;
    write->name = (char*)malloc(33);
    write->name = "write";
    write->returnType = int_type;
    write->para = (struct paraNode*)malloc(sizeof(struct paraNode));
    write->para->name = NULL;
    write->para->Type = int_type;
    write->para->next = NULL;
    write->next = FunList;
    FunList = write;

}