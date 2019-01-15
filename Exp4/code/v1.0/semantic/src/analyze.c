#include "analyze.h"

void analyze(struct Node* root){
    if (root == NULL)
        return;

    #ifdef DIAG_MODE
    printf("Analyze %s at line %d: \n", root->name, root->lineno);
    #endif
    
    //局部变量定义
    if (!strcmp(root->name, "Def")){
        struct Node* specifier = root->child;
        struct Node* dec = specifier->sibling->child;
        while(1){
            addExtNode(specifier, dec->child, 0, 1);
            if (dec->sibling != NULL)
                dec = dec->sibling->sibling->child;
            else
                break;
        }
    }

    #ifdef DIAG_MODE
    printf("#\n");
    #endif

    if (!strcmp(root->name, "ExtDef")){
        //全局变量定义
        if (!strcmp(root->child->sibling->name, "ExtDecList"))
            addExtNode(root->child, root->child->sibling->child, 0, 0);

        //函数声明或定义
        else if (!strcmp(root->child->sibling->name, "FunDec")){
            addFunNode(root);

            struct Node* compst = root->child->sibling->sibling;
            //函数定义
            if (!strcmp(compst->name, "CompSt")){
                //检查返回值是否相符
                struct Node* stmtlist = compst->child->sibling->sibling;
                checkReturn(stmtlist, FunList->returnType);  //刚加入的函数在链表头部

                //将形参加入全局变量表(函数声明不需要)
                struct Node* varlist = root->child->sibling->child->sibling->sibling;
                if (!strcmp(varlist->name, "VarList")){
                    int repe = 1;
                    while (repe == 1){
                        struct Node* vardec = varlist->child->child->sibling;

                        //get name
                        struct Node* nont = vardec->child;
                        while (strcmp(nont->name, "ID"))
                            nont = nont->child;
                        char* name = (char*)malloc(33);
                        strcpy(name, nont->id_name);

                        //add to ExtList
                        addExtNode(varlist->child->child, vardec, 1, 1);
                        
                        if (varlist->child->sibling != NULL)
                            varlist = varlist->child->sibling->sibling;
                        else
                            repe = 0;
                    }
                }
            }
        }
    }

    #ifdef DIAG_MODE
    printf("#\n");
    #endif

    //结构体类型变量定义
    if (!strcmp(root->name, "Specifier")
        &&!strcmp(root->child->name, "StructSpecifier")
        &&!strcmp(root->child->child->sibling->name, "OptTag")
        &&root->child->child->sibling->child != NULL
        &&!strcmp(root->child->child->sibling->child->name, "ID"))
    {
        addStructNode(root);
    }
    
    #ifdef DIAG_MODE
    printf("#\n");
    #endif

    //表达式
    if (!strcmp(root->name, "Exp")){
        if(root->child->sibling == NULL){
            //变量引用
            if (!strcmp(root->child->name, "ID")){ 
                struct ExtSListNode* p = ExtSList;
                struct ExtSListNode* def = NULL;
                while(p != NULL){
                    if (!strcmp(root->child->id_name, p->name)){
                        def = p;
                        break;
                    }
                    p = p->next;
                }
                if (def == NULL)
                    addErrorNode(1, root->lineno, root->child->id_name);
            }
            //NOT
            else if (!strcmp(root->child->name, "NOT")){
                //int allowed only
                struct TypeNode* p = getExpType(root->child->sibling);
                if (p->type != 0)
                    addErrorNode(7, root->lineno, NULL);
            }
            //MINUS
            else if (!strcmp(root->child->name, "MINUS")){
                //int or float allowed
                struct TypeNode* p = getExpType(root->child->sibling);
                if (p->type == 2)
                    addErrorNode(7, root->lineno, NULL);
            }
        }
        else{
            //函数调用
            if (!strcmp(root->child->name, "ID")){
                struct FunListNode* p = FunList;
                struct FunListNode* def = NULL;
                while(p != NULL){
                    if (!strcmp(root->child->id_name, p->name)){
                        def = p;
                        break;
                    }
                    p = p->next;
                }
                if (def == NULL){//未找到定义
                    struct ExtSListNode* var = ExtSList;
                    int flag = 0;
                    while (var != NULL){
                        if (!strcmp(root->child->id_name, var->name)){
                            addErrorNode(11, root->lineno, root->child->id_name);
                            flag = 1;
                            break;
                        }
                        var = var->next;
                    }
                    if (flag == 0)
                        addErrorNode(2, root->lineno,root->child->id_name);
                }
                else{
                    //检查形参是否一致
                    struct Node* args = root->child->sibling->sibling;
                    if (checkPara(def, args) != 1)
                        addErrorNode(9, root->lineno, root->child->id_name);
                }
            }
            //四则运算和RELOP
            else if (!strcmp(root->child->sibling->name, "PLUS")
                ||!strcmp(root->child->sibling->name, "MINUS")
                ||!strcmp(root->child->sibling->name, "STAR")
                ||!strcmp(root->child->sibling->name, "DIV")
                ||!strcmp(root->child->sibling->name, "RELOP"))
            {
                struct TypeNode* p = getExpType(root->child);
                struct TypeNode* q = getExpType(root->child->sibling->sibling);
                if (p->type == 2
                    ||q->type == 2
                    ||TypeCheck(p, q) == 0)
                    addErrorNode(7, root->lineno, NULL);
            }
            //逻辑运算
            else if (!strcmp(root->child->sibling->name, "AND")
                ||!strcmp(root->child->sibling->name, "OR"))
            {
                //int allowed only
                struct TypeNode* p = getExpType(root->child);
                struct TypeNode* q = getExpType(root->child->sibling->sibling);
                if (p->type != 1||q->type != 1)
                    addErrorNode(7, root->lineno, NULL);
            }
            //赋值表达式
            else if (!strcmp(root->child->sibling->name, "ASSIGNOP")){
                struct TypeNode* p = getExpType(root->child);
                struct TypeNode* q = getExpType(root->child->sibling->sibling);
                if (TypeCheck(p, q) == 0)
                    addErrorNode(5, root->lineno, NULL);

                //检查等号左边是否是左值表达式
                int leftvalue = 0;
                struct Node* exp = root->child;
                if (exp->child->sibling == NULL){
                    if (!strcmp(exp->child->name, "ID")) //ID
                        leftvalue = 1;
                }
                else if (!strcmp(exp->child->sibling->name, "LB")||!strcmp(exp->child->sibling->name, "DOT")) //struct or array
                        leftvalue = 1;
                if (leftvalue == 0)
                    addErrorNode(6, root->lineno, NULL);
            }
            //数组访问
            else if (!strcmp(root->child->sibling->name, "LB")){
                struct Node* num = root->child->sibling->sibling;
                struct TypeNode* t = getExpType(num);

                if (t == NULL||t->type != 0)
                    addErrorNode(12, num->lineno, NULL);

                struct TypeNode* t2 = getExpType(root->child);
                if (t2 == NULL||t2->Type == NULL){
                    //get name
                    struct Node* id = root->child->child;
                    while (strcmp(id->name, "ID"))
                        id = id->child;
                    char* buf = (char*)malloc(33);
                    strcpy(buf, id->id_name);
                    //not an array
                    addErrorNode(10, num->lineno, buf);
                }
                    
            }
            //结构体访问
            else if (!strcmp(root->child->sibling->name, "DOT")){
                struct TypeNode* t = getExpType(root->child);
                if (t == NULL||t->Type == NULL||t->size != -1)
                    addErrorNode(13, root->lineno, NULL);

                int flag2 = 0;
                struct Node* id = root->child->sibling->sibling;

                while (t != NULL){
                    if (!strcmp(id->id_name, t->name))
                        flag2 = 1;
                    t = t->next;
                }

                if (flag2 == 0){
                    //get name
                    struct Node* id = root->child->child;
                    while (strcmp(id->name, "ID"))
                        id = id->child;
                    char* buf = (char*)malloc(33);
                    strcpy(buf, id->id_name);
                    //not a struct
                    addErrorNode(14, root->lineno, buf);
                }
            }
        }
    }

    #ifdef DIAG_MODE
    printf("#\n");
    #endif
    
    //条件语句
    if (!strcmp(root->name, "IF")||!strcmp(root->name, "WHILE")){
        //int allowed only
        struct TypeNode* p = getExpType(root->sibling->sibling);
        if (p->type != 0)
            addErrorNode(7, root->lineno, NULL);
    }

    #ifdef DIAG_MODE
    printf("#\n");
    #endif

    //函数返回式
    if (!strcmp(root->name, "RETURN")){
        struct TypeNode* expType = getExpType(root->sibling);
        if (TypeCheck(expType, root->extra) == 0)
            addErrorNode(8, root->lineno, NULL);
    }

    #ifdef DIAG_MODE
    printf("#\n");
    #endif


    analyze(root->child);
    analyze(root->sibling);
}





