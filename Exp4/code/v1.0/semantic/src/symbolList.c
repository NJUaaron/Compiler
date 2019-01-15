#include "symbolList.h"

struct ExtSListNode* ExtSList = NULL;
struct StructListNode* StructList = NULL;

void addExtNode(struct Node* specifier, struct Node* vardec, int pa, int fu){
    struct ExtSListNode* node = (struct ExtSListNode*)malloc(sizeof(struct ExtSListNode));

    //get type
    node->Type = getType2(specifier, vardec);

    //get name
    struct Node* nont = vardec->child;
    while (strcmp(nont->name, "ID"))
        nont = nont->child;
    char* buf = (char*)malloc(33);
    strcpy(buf, nont->id_name);
    node->name = buf;

    //get IR no
    node->no = new_temp();

    node->inParam = pa;
    node->inFunc = fu;

    //check redefine
    struct ExtSListNode* p = ExtSList;
    while (p != NULL){
        if (!strcmp(node->name, p->name)){
            addErrorNode(3, specifier->lineno, node->name);
            break;
        }
        p = p->next;
    }
    struct StructListNode* q = StructList;
    while (q != NULL){
        if (!strcmp(node->name, q->name)){
            addErrorNode(3, specifier->lineno, node->name);
            break;
        }
        q = q->next;
    }

    //check type match
    if (vardec->sibling != NULL){  //initialize in definition
        struct TypeNode* q = getExpType(vardec->sibling->sibling);
        if (TypeCheck(node->Type, q) == 0)
            addErrorNode(5, vardec->lineno, NULL);
    }

    //add to external symbol list
    node->next = ExtSList;
    ExtSList = node;
}

struct ExtSListNode* InExtList(char* name){
    struct ExtSListNode* p = ExtSList;
    while (p != NULL){
        if (!strcmp(name, p->name))
            break;
        p = p->next;
    }
    return p;
}

struct TypeNode* getType(struct Node* specifier){
    struct TypeNode* ret = NULL;
    struct TypeNode* tail = NULL;

    if (!strcmp(specifier->child->name, "TYPE")){//int float
        struct TypeNode* node = (struct TypeNode*)malloc(sizeof(struct TypeNode));

        node->name = NULL;
        node->size = -1;
        if (!strcmp(specifier->child->id_name, "int"))
            node->type = 0;
        else
            node->type = 1;
        node->Type = NULL;

        node->next = ret;
        ret = node;
    }
    else{//struct
        if (!strcmp(specifier->child->child->sibling->name, "OptTag")){
            struct Node* deflist = specifier->child->child->sibling->sibling->sibling;
            while(deflist->child != NULL){
                struct Node* declist = deflist->child->child->sibling;

                int repe2 = 1;  //repetition
                while(repe2){
                    struct Node* vardec = declist->child->child;

                    struct TypeNode* node = (struct TypeNode*)malloc(sizeof(struct TypeNode));                    
                    node->Type = NULL;
                    
                    while(vardec->child->sibling != NULL){
                        struct TypeNode* anode = (struct TypeNode*)malloc(sizeof(struct TypeNode));
                        anode->name = NULL;
                        anode->type = 2;
                        anode->next = NULL;
                        anode->size = vardec->child->sibling->sibling->int_val;


                        if (tail == NULL){

                        }
                        anode->Type = node->Type;
                        node->Type = anode;

                        vardec = vardec->child;
                    }
                    struct TypeNode* ptr = node->Type;
                    if (ptr == NULL){
                        node->Type = getType(deflist->child->child);
                    }
                    else{
                        while (ptr->Type != NULL)
                            ptr = ptr->Type;
                        ptr->Type = getType(deflist->child->child);
                    }

                    char* buf = (char*)malloc(33);
                    strcpy(buf, vardec->child->id_name);
                    node->name = buf;
                    node->type = 2;
                    node->size = -1;
                    node->next = NULL;

                    //定义时不能对域进行初始化
                    if (vardec->sibling != NULL&&!strcmp(vardec->sibling->name, "ASSIGNOP"))
                        addErrorNode(15, vardec->lineno, node->name);

                    if (tail == NULL){         
                        ret = node;
                        tail = node;
                    }
                    else{
                        tail->next = node;
                        tail = node;
                    }


                    if(declist->child->sibling == NULL)
                        repe2 = 0;
                    else
                        declist = declist->child->sibling->sibling;
                }
                
                deflist = deflist->child->sibling;
            }   
        }
        else{//结构体应已在前面定义过
            //查表
            struct StructListNode* p = StructList;
            struct Node* id = specifier->child->child->sibling->child;
            while (p != NULL){
                if (!strcmp(p->name, id->id_name)){
                    return p->Type;
                }
                p = p->next;
            }
            addErrorNode(17, specifier->lineno, id->id_name);
        }
    }

    return ret;
}

struct TypeNode* getType2(struct Node* specifier, struct Node* vardec){
    struct TypeNode* ret = NULL;
    struct NodeType* tail = NULL;
    
    while(vardec->child->sibling != NULL){
        struct TypeNode* anode = (struct TypeNode*)malloc(sizeof(struct TypeNode));
        anode->name = NULL;
        anode->type = 2;
        anode->next = NULL;
        anode->size = vardec->child->sibling->sibling->int_val;

        anode->Type = ret;
        ret = anode;

        vardec = vardec->child;
    }
    struct TypeNode* ptr = ret;
    if (ptr == NULL){
        ret = getType(specifier);
    }
    else{
        while (ptr->Type != NULL)
            ptr = ptr->Type;
        ptr->Type = getType(specifier);
    }
    return ret;
}

struct TypeNode* getExpType(struct Node* exp){
    
    if (exp == NULL)
        return NULL;
    
    //int
    if (!strcmp(exp->child->name, "INT")){
        struct TypeNode* t =  (struct TypeNode*)malloc(sizeof(struct TypeNode));
        t->name = NULL;
        t->size = -1;
        t->type = 0;
        t->Type = NULL;
        return t;
    }
    //float
    else if (!strcmp(exp->child->name, "FLOAT")){
        struct TypeNode* t =  (struct TypeNode*)malloc(sizeof(struct TypeNode));
        t->name = NULL;
        t->size = -1;
        t->type = 1;
        t->Type = NULL;
        return t;
    }
    else if (!strcmp(exp->child->name, "ID")){
        //vaiable
        if (exp->child->sibling == NULL){
            struct ExtSListNode* p = ExtSList;
            while (p != NULL){
                if(!strcmp(exp->child->id_name, p->name))
                    return p->Type;
                p = p->next;
            }
        }
        //function
        else{
            struct FunListNode* p = FunList;
            while (p != NULL){
                if(!strcmp(exp->child->id_name, p->name))
                    return p->returnType;
                p = p->next;
            }
        }
        return NULL;
    }
    //logical expression
    else if (!strcmp(exp->child->sibling->name, "AND")
        ||!strcmp(exp->child->sibling->name, "OR")
        ||!strcmp(exp->child->sibling->name, "RELOP")
        ||!strcmp(exp->child->name, "NOT"))
    {
        struct TypeNode* t =  (struct TypeNode*)malloc(sizeof(struct TypeNode));
        t->name = NULL;
        t->size = -1;
        t->type = 0;
        t->Type = NULL;
        return t;
    }
    //array
    else if (!strcmp(exp->child->sibling->name, "LB")){
        struct TypeNode* t = getExpType(exp->child);
        if (t == NULL||t->Type == NULL){
            //addErrorNode(10, exp->lineno);
            return t;
        }
        return t->Type;
    }
    //struct
    else if (!strcmp(exp->child->sibling->name, "DOT")){
        struct TypeNode* t = getExpType(exp->child);
        if (t == NULL||t->Type == NULL||t->size != -1){
            //addErrorNode(13, exp->lineno);
            return t;
        }
        struct Node* id = exp->child->sibling->sibling;
        while (t != NULL){
            if (!strcmp(id->id_name, t->name))
                return t->Type;
            t = t->next;
        }
        //addErrorNode(14, exp->lineno);
        return t;
    }
    else if (!strcmp(exp->child->name, "LP")||!strcmp(exp->child->name, "MINUS"))
        return getExpType(exp->child->sibling);
    else
        return getExpType(exp->child);
    
}

int getTypeWidth(struct TypeNode* Type){
    if (Type == NULL)
        return 0;
    else if (Type->size != -1)
        return Type->size * getTypeWidth(Type->Type);
    int width = 0;
    for (struct TypeNode* ptr = Type; ptr != NULL; ptr = ptr->next){
        if (ptr->type != 2)
            width += 4;
        else
            width += getTypeWidth(ptr->Type);
    }
    return width;
}

int TypeCheck(struct TypeNode* a, struct TypeNode* b){
    if (a == NULL || b == NULL)
        return 0;
    //数组维度相同即可
    /*
    if (a->size != b->size)
        return 0;
    */
    if (a->type != b->type)
        return 0;

    if (a->next == NULL && b->next != NULL)
        return 0;
    
    if (a->next != NULL && b->next == NULL)
        return 0;
    
    if (a->Type != NULL && b->Type != NULL)
        if (TypeCheck(a->Type, b->Type) == 0)
            return 0;
    
    if (a->next != NULL && b->next != NULL)
        if (TypeCheck(a->next, b->next) == 0)
            return 0;

    return 1;
}

void printSt(struct TypeNode* Type, int depth){
    if (Type == NULL)
        return;
    
    for(int i = 0; i < depth; i++)    //retract
        printf("  ");
    printf("%d, %d", Type->size, Type->type);
    if (Type->name != NULL)
        printf(", %s",Type->name);
    printf("\n");
    

    printSt(Type->Type, depth + 1);
    printSt(Type->next, depth);
}

void addStructNode(struct Node* specifier){
    struct StructListNode* node = (struct StructListNode*)malloc(sizeof(struct StructListNode));

    node->Type = getType(specifier);

    char* buf = (char*)malloc(33);
    strcpy(buf, specifier->child->child->sibling->child->id_name);
    node->name = buf;

    //check repitition
    struct StructListNode* p = StructList;
    while (p != NULL){
        if (!strcmp(p->name, node->name)){
            addErrorNode(16, specifier->lineno, node->name);
            break;
        }
        p = p->next;
    }
    struct ExtSListNode* q = ExtSList;
    while (q != NULL){
        if (!strcmp(node->name, q->name)){
            addErrorNode(16, specifier->lineno, node->name);
            break;
        }
        q = q->next;
    }

    //add to struct list
    node->next = StructList;
    StructList = node;
}