#include "translate.h"

struct InterCode* translate_Exp(struct Node* exp, int place){
    #ifdef T_DIAG_MODE
    printf("translate_Exp: BEGIN  lineno:%d\n", exp->lineno);
    #endif

    if (exp->child->sibling == NULL){
        //INT
        if (!strcmp(exp->child->name, "INT")){
            return c_assign(place, 1, exp->child->int_val);
        }
        //no FLOAT
        if (!strcmp(exp->child->name, "FLOAT")){
            printf("Cannot translate: Couldn't receive float.\n");
            return c_assign(place, 1, (int)(exp->child->float_val));
        }
        //ID
        else if (!strcmp(exp->child->name, "ID")){
            struct ExtSListNode* vari = InExtList(exp->child->id_name);
            return c_assign(place, 0, vari->no);
        }
    }
    else{
        //PARENTHESIS
        if (!strcmp(exp->child->name, "LP")){
            return translate_Exp(exp->child->sibling, place);
        }
        //ASSIGNOP
        if (!strcmp(exp->child->sibling->name, "ASSIGNOP")){
            //考察左值类型
            //variable
            if (!strcmp(exp->child->child->name, "ID")){
                #ifdef T_DIAG_MODE
                printf("    variable ASSIGNOP: BEGIN\n");
                #endif

                struct ExtSListNode* vari = InExtList(exp->child->child->id_name);
                struct InterCode* code1 = translate_Exp(exp->child->sibling->sibling, vari->no);
                struct InterCode* code2 = c_assign(place, 0, vari->no);

                #ifdef T_DIAG_MODE
                printf("    variable ASSIGNOP: END\n");
                #endif

                return bindCode(code1, code2);
            }
            //left array
            else if (!strcmp(exp->child->child->sibling->name, "LB")){
                #ifdef T_DIAG_MODE
                printf("    left array ASSIGNOP: BEGIN\n");
                #endif

                //exp -> exp0 (= exp3) -> exp1 LB exp2 RB (= exp)
                struct Node* exp1 = exp->child->child;
                struct Node* exp2 = exp1->sibling->sibling;
                struct Node* exp3 = exp->child->sibling->sibling;

                int width = getTypeWidth(getExpType(exp));    //exp's width

                //PART1
                struct InterCode* part1 = NULL;     //calculate offset byte
                int t1 = new_temp();    //record offset byte
                int t4 = new_temp();    //data width

                if (!strcmp(exp2->child->name, "ID")&&exp2->child->sibling == NULL){
                    struct ExtSListNode* vari = InExtList(exp2->child->id_name);
                    struct InterCode *code1 = c_assign(t4, 1, width);
                    struct InterCode* code2 = c_operation(t1, 0, vari->no, "*", 0, t4);
                    part1 = bindCode(code1, code2);
                }
                else{
                    int t3 = new_temp();
                    struct InterCode* code1 = translate_Exp(exp2, t3);
                    struct InterCode* code2 = c_assign(t4, 1, width);
                    struct InterCode* code3 = c_operation(t1, 0, t3, "*", 0, t4);
                    part1 = bindCode(code1, bindCode(code2, code3));
                }

                //PART2
                struct InterCode* part2 = NULL;     //calculate exp0's addr
                int t2 = new_temp();    //record exp0's addr

                if (!strcmp(exp1->child->name, "ID")&&exp1->child->sibling == NULL){
                    struct ExtSListNode* vari = InExtList(exp1->child->id_name);
                    if (vari->inParam == 1)
                        part2 = c_operation(t2, 0, vari->no, "+", 0, t1);
                    else{
                        int t5 = new_temp();
                        struct InterCode *code1 = c_assign(t5, 3, vari->no);
                        struct InterCode* code2 = c_operation(t2, 0, t5, "+", 0, t1);
                        part2 = bindCode(code1, code2);
                    }
                }
                else{
                    int t3 = new_temp();
                    struct InterCode* code1 = translate_addr(exp1, t3);    //exp1's addr
                    struct InterCode* code2 = c_operation(t2, 0, t3, "+", 0, t1);    //exp0's addr
                    part2 = bindCode(code1, code2);
                }

                //PART3
                struct InterCode* part3 = NULL;    //exp3's value
                if (!strcmp(exp3->child->name, "ID")&&exp3->child->sibling == NULL){
                    struct ExtSListNode* vari = InExtList(exp3->child->id_name);
                    part3 = c_writemem(t2, 0, vari->no);
                }
                else{
                    int t3 = new_temp();
                    struct InterCode* code1 = translate_Exp(exp3, t3);
                    struct InterCode* code2 = c_writemem(t2, 0, t3);
                    part3 = bindCode(code1, code2);
                }

                struct InterCode* part4 = c_assign(place, 3, t2);

                #ifdef T_DIAG_MODE
                printf("    left array ASSIGNOP: END\n");
                #endif

                return bindCode(part1, bindCode(part2, bindCode(part3, part4)));
            }
            //left struct
            else if (!strcmp(exp->child->child->sibling->name, "DOT")){
                #ifdef T_DIAG_MODE
                printf("    left struct ASSIGNOP: BEGIN\n");
                #endif

                //exp -> exp0 (= exp2) -> exp1 DOT ID (= exp)
                struct Node* exp1 = exp->child->child;
                struct Node* exp2 = exp->child->sibling->sibling;
                struct Node* id = exp1->sibling->sibling;

                struct TypeNode* exp1_type = getExpType(exp1);
                int offset = 0;
                for (struct TypeNode* p = exp1_type; strcmp(p->name, id->id_name); p = p->next){
                    if (p->type != 2)
                        offset += 4;
                    else
                        offset += getTypeWidth(p->Type);
                }

                struct InterCode* part1 = NULL;     //calculate exp0's addr
                int t1 = new_temp();    //record exp0's addr

                if (!strcmp(exp1->child->name, "ID")){
                    struct ExtSListNode* vari = InExtList(exp1->child->id_name);
                    if (offset == 0){
                        if (vari->inParam == 1)
                            part1 = c_assign(t1, 0 ,vari->no);
                        else
                            part1 = c_assign(t1, 2 ,vari->no);
                    }
                    else{
                        if (vari->inParam == 1)
                            part1 = c_operation(t1, 0 ,vari->no, "+", 1, offset);
                        else{
                            int tmp = new_temp();
                            struct InterCode *code1 = c_assign(tmp, 2, vari->no);
                            struct InterCode *code2 = c_operation(t1, 0, tmp, "+", 1, offset);
                            part1 = bindCode(code1, code2);
                        }
                    }
                }
                else{
                    struct InterCode* code1 = translate_addr(exp1, t1);    //exp1's addr
                    if (offset == 0){
                        part1 = translate_addr(exp1, t1);
                    }
                    else{
                        int t2 = new_temp();
                        struct InterCode* code1 = translate_addr(exp1, t2);
                        struct InterCode* code2 = c_operation(t1, 0, t2, "+", 1, offset);    //exp's addr
                        part1 = bindCode(code1, code2);
                    }
                } 

                struct InterCode* part2 = NULL;    //exp2's value
                if (!strcmp(exp2->child->name, "ID")){
                    struct ExtSListNode* vari = InExtList(exp2->child->id_name);
                    part2 = c_writemem(t1, 0, vari->no);
                }
                else{
                    int t3 = new_temp();
                    struct InterCode* code1 = translate_Exp(exp2, t3);
                    struct InterCode* code2 = c_writemem(t1, 0, t3);
                    part2 = bindCode(code1, code2);
                }

                struct InterCode* part3 = c_assign(place, 3, t1);

                #ifdef T_DIAG_MODE
                printf("    left struct ASSIGNOP: END\n");
                #endif

                return bindCode(part1, bindCode(part2, part3));
            }
        }

        //四则运算
        else if (!strcmp(exp->child->sibling->name, "PLUS")||!strcmp(exp->child->sibling->name, "MINUS")){
            struct Node* exp1 = exp->child;
            struct Node* exp2 = exp1->sibling->sibling;
            char* op = NULL;

            if (!strcmp(exp->child->sibling->name, "PLUS"))
                op = "+";
            else
                op = "-";

            if (!strcmp(exp1->child->name, "ID") && exp1->child->sibling == NULL){
                if (!strcmp(exp2->child->name, "INT")){
                    struct ExtSListNode* vari = InExtList(exp1->child->id_name);
                    return c_operation(place, 0, vari->no, op, 1, exp2->child->int_val);
                }
                else if (!strcmp(exp2->child->name, "ID") && exp2->child->sibling == NULL){
                    struct ExtSListNode* vari1 = InExtList(exp1->child->id_name);
                    struct ExtSListNode* vari2 = InExtList(exp2->child->id_name);
                    return c_operation(place, 0, vari1->no, op, 0, vari2->no);
                }
                else{
                    int t1 = new_temp();
                    struct ExtSListNode* vari = InExtList(exp1->child->id_name);
                    struct InterCode* code1 = translate_Exp(exp2, t1);
                    struct InterCode* code2 = c_operation(place, 0, vari->no, op, 0, t1);
                    return bindCode(code1, code2);
                }
            }
            else{
                if (!strcmp(exp2->child->name, "INT")){
                    int t1 = new_temp();
                    struct InterCode* code1 = translate_Exp(exp1, t1);
                    struct InterCode* code2 = c_operation(place, 0, t1, op, 1, exp2->child->int_val);
                    return bindCode(code1, code2);
                }
                else if (!strcmp(exp2->child->name, "ID") && exp2->child->sibling == NULL){
                    int t1 = new_temp();
                    struct ExtSListNode* vari = InExtList(exp2->child->id_name);
                    struct InterCode* code1 = translate_Exp(exp1, t1);
                    struct InterCode* code2 = c_operation(place, 0, t1, op, 0, vari->no);
                    return bindCode(code1, code2);
                }
                else{
                    int t1 = new_temp();
                    int t2 = new_temp();
                    struct InterCode* code1 = translate_Exp(exp1, t1);
                    struct InterCode* code2 = translate_Exp(exp2, t2);
                    struct InterCode* code3 = c_operation(place, 0, t1, op, 0, t2);
                    return bindCode(code1, bindCode(code2, code3));
                }
            }
        }
        else if (!strcmp(exp->child->sibling->name, "STAR")||!strcmp(exp->child->sibling->name, "DIV")){
            struct Node* exp1 = exp->child;
            struct Node* exp2 = exp1->sibling->sibling;
            char* op = NULL;

            if (!strcmp(exp->child->sibling->name, "STAR"))
                op = "*";
            else
                op = "/";

            if (!strcmp(exp1->child->name, "ID") && exp1->child->sibling == NULL){
                if (!strcmp(exp2->child->name, "ID") && exp2->child->sibling == NULL){
                    struct ExtSListNode* vari1 = InExtList(exp1->child->id_name);
                    struct ExtSListNode* vari2 = InExtList(exp2->child->id_name);
                    return c_operation(place, 0, vari1->no, op, 0, vari2->no);
                }
                else{
                    int t1 = new_temp();
                    struct ExtSListNode* vari = InExtList(exp1->child->id_name);
                    struct InterCode* code1 = translate_Exp(exp2, t1);
                    struct InterCode* code2 = c_operation(place, 0, vari->no, op, 0, t1);
                    return bindCode(code1, code2);
                }
            }
            else{
                if (!strcmp(exp2->child->name, "ID") && exp2->child->sibling == NULL){
                    int t1 = new_temp();
                    struct ExtSListNode* vari = InExtList(exp2->child->id_name);
                    struct InterCode* code1 = translate_Exp(exp1, t1);
                    struct InterCode* code2 = c_operation(place, 0, t1, op, 0, vari->no);
                    return bindCode(code1, code2);
                }
                else{
                    int t1 = new_temp();
                    int t2 = new_temp();
                    struct InterCode* code1 = translate_Exp(exp1, t1);
                    struct InterCode* code2 = translate_Exp(exp2, t2);
                    struct InterCode* code3 = c_operation(place, 0, t1, op, 0, t2);
                    return bindCode(code1, bindCode(code2, code3));
                }
            }
        }
        //MINUS
        else if (!strcmp(exp->child->name, "MINUS")){
            struct Node* exp1 = exp->child->sibling;
            if (!strcmp(exp1->child->name, "INT"))
                return c_assign(place, 1, -(exp1->child->int_val));
            else if (!strcmp(exp1->child->name, "FLOAT")){
                printf("Cannot translate: Couldn't receive float.\n");
                return c_assign(place, 1, -(int)(exp->child->float_val));
            }
            else{
                int t1 = new_temp();
                int t2 = new_temp();
                struct InterCode* code1 = translate_Exp(exp->child->sibling, t1);
                struct InterCode *code2 = c_assign(t2, 1, 0);
                struct InterCode *code3 = c_operation(place, 0, t2, "-", 0, t1);
                return bindCode(code1, bindCode(code2, code3));
            }
        }
        //Condition
        else if (!strcmp(exp->child->sibling->name, "RELOP")
            ||!strcmp(exp->child->sibling->name, "AND")
            ||!strcmp(exp->child->sibling->name, "OR")
            ||!strcmp(exp->child->name, "NOT"))
        {
            int label1 = new_label();
            int label2 = new_label();
            struct InterCode* code1 = c_assign(place, 1, 0);
            struct InterCode* code2 = translate_Cond(exp, label1, label2);
            struct InterCode* code3 = c_label(label1);
            struct InterCode* code4 = c_assign(place, 1, 1);
            struct InterCode* code5 = c_label(label2);
            struct InterCode* tmp = bindCode(code1, bindCode(code2, code3));
            return bindCode(tmp, bindCode(code4, code5));
        }
        //Function
        else if (!strcmp(exp->child->name, "ID")){
            if (!strcmp(exp->child->sibling->sibling->name, "RP")){
                if (!strcmp(exp->child->id_name, "read"))
                    return c_read(place);
                return c_call(place, exp->child->id_name);
            }
            else{
                #ifdef T_DIAG_MODE
                printf("    Function: BEGIN\n");
                #endif

                struct argList* arg_list = (struct argList*)malloc(sizeof(struct argList));    //链表头
                arg_list->next = NULL;
                struct InterCode* code1 = translate_Args(exp->child->sibling->sibling, arg_list);
                if (!strcmp(exp->child->id_name, "write"))
                    return bindCode(code1, c_write(arg_list->next->type, arg_list->next->val));
                struct InterCode* code2 = NULL;
                for (struct argList* p = arg_list->next; p != NULL; p= p->next)
                    code2 = bindCode(code2, c_arg(p->type, p->val));
                
                #ifdef T_DIAG_MODE
                printf("    Function: END\n");
                #endif

                return bindCode(code1, bindCode(code2, c_call(place, exp->child->id_name)));
            }
        }    

        //right array
        else if (!strcmp(exp->child->sibling->name, "LB")){
            #ifdef T_DIAG_MODE
            printf("    right array: BEGIN\n");
            #endif

            //exp -> exp1 LB exp2 RB
            struct Node* exp1 = exp->child;
            struct Node* exp2 = exp1->sibling->sibling;

            int width = getTypeWidth(getExpType(exp));    //exp's width

            //PART1
            struct InterCode* part1 = NULL;     //calculate offset byte
            int t1 = new_temp();    //record offset byte
            int t4 = new_temp();    //data width

            if (!strcmp(exp2->child->name, "ID")&&exp2->child->sibling == NULL){
                struct ExtSListNode* vari = InExtList(exp2->child->id_name);
                struct InterCode *code1 = c_assign(t4, 1, width);
                struct InterCode* code2 = c_operation(t1, 0, vari->no, "*", 0, t4);
                part1 = bindCode(code1, code2);
            }
            else{
                int t3 = new_temp();
                struct InterCode* code1 = translate_Exp(exp2, t3);
                struct InterCode *code2 = c_assign(t4, 1, width);
                struct InterCode* code3 = c_operation(t1, 0, t3, "*", 0, t4);
                part1 = bindCode(code1, bindCode(code2, code3));
            }

            //PART2
            struct InterCode* part2 = NULL;     //calculate exp0's addr
            int t2 = new_temp();    //record exp0's addr

            if (!strcmp(exp1->child->name, "ID")&&exp1->child->sibling == NULL){
                struct ExtSListNode* vari = InExtList(exp1->child->id_name);
                if (vari->inParam == 1)
                    part2 = c_operation(t2, 0, vari->no, "+", 0, t1);
                else{
                    int tmp = new_temp();
                    struct InterCode *code1 = c_assign(tmp, 3, vari->no);
                    struct InterCode *code2 = c_operation(t2, 0, tmp, "+", 0, t1);
                    part2 = bindCode(code1, code2);
                }
            }
            else{
                int t3 = new_temp();
                struct InterCode* code1 = translate_addr(exp1, t3);    //exp1's addr
                struct InterCode* code2 = c_operation(t2, 0, t3, "+", 0, t1);    //exp0's addr
                part2 = bindCode(code1, code2);
            }

            struct InterCode* part3 = c_assign(place, 3, t2);

            #ifdef T_DIAG_MODE
            printf("    right array: END\n");
            #endif

            return bindCode(part1, bindCode(part2, part3));
        }

        //right struct
        else if (!strcmp(exp->child->sibling->name, "DOT")){

            //exp -> exp1 DOT ID
            struct Node* exp1 = exp->child;
            struct Node* id = exp1->sibling->sibling;

            struct TypeNode* exp1_type = getExpType(exp1);
            int offset = 0;
            for (struct TypeNode* p = exp1_type; strcmp(p->name, id->id_name); p = p->next){
                if (p->type != 2)
                    offset += 4;
                else
                    offset += getTypeWidth(p->Type);
            }
            if (!strcmp(exp1->child->name, "ID")){
                struct ExtSListNode* vari = InExtList(exp1->child->id_name);
                if (offset == 0){
                    if (vari->inParam == 1)
                        return c_assign(place, 3 ,vari->no);
                    else
                        return c_assign(place, 0 ,vari->no);
                }
                else{
                    int t1 = new_temp();
                    struct InterCode* code1 = NULL;

                    if (vari->inParam == 1)
                        code1 = c_operation(t1, 0 ,vari->no, "+", 1, offset);
                    else
                        code1 = c_operation(t1, 2 ,vari->no, "+", 1, offset);

                    struct InterCode* code2 = c_assign(place, 3, t1);
                    return bindCode(code1, code2);
                }
            }
            else{
                int t1 = new_temp();
                struct InterCode* code1 = translate_addr(exp1, t1);    //exp1's addr
                if (offset == 0){
                    struct InterCode* code2 = c_assign(place, 3, t1);
                    return bindCode(code1, code2);
                }
                else{
                    int t2 = new_temp();
                    struct InterCode* code2 = c_operation(t2, 0, t1, "+", 1, offset);    //exp's addr
                    struct InterCode* code3 = c_assign(place, 3, t2);
                    return bindCode(code1, bindCode(code2, code3));
                }
            }      
        }
    }
}

struct InterCode* translate_Args(struct Node* args, struct argList* arg_list){
    #ifdef T_DIAG_MODE
    printf("translate_Args: BEGIN\n");
    #endif

    struct Node* exp = args->child;
    struct InterCode* part1 = NULL;

    //arg_list = t1 + arg_list
    struct argList* node = (struct argList*)malloc(sizeof(struct argList));

    struct TypeNode* expType = getExpType(exp);
    if (expType->type == 2){     //array or struct
        if (!strcmp(exp->child->name, "ID")){
            struct ExtSListNode* vari = InExtList(exp->child->id_name);
            if (vari->inParam == 0)
                node->type = 2;
            else
                node->type = 0;
            node->val = vari->no;
        }
        else{
            int t1 = new_temp();
            part1 = translate_addr(exp, t1);
            node->type = 0;
            node->val = t1;
        }
    }
    else{
        if (!strcmp(exp->child->name, "INT")){
            node->type = 1;
            node->val = exp->child->int_val;
        }
        else if (!strcmp(exp->child->name, "ID") && exp->child->sibling == NULL){
            struct ExtSListNode* vari = InExtList(exp->child->id_name);
            node->type = 0;
            node->val = vari->no;
        }
        else{
            int t1 = new_temp();
            part1 = translate_Exp(exp, t1);
            node->type = 0;
            node->val = t1;
        }
    }

    node->next = NULL;
    struct argList* p = arg_list;
    while (p->next != NULL)
        p = p->next;
    p->next = node;

    //Args -> Exp
    if (exp->sibling == NULL)
        return part1;
    //Args -> Exp COMMA Args
    else{ 
        struct InterCode* part2 = translate_Args(exp->sibling->sibling, arg_list);
        return bindCode(part1, part2);
    }
}

struct InterCode* translate_addr(struct Node* exp, int place){
    //ID
    if (!strcmp(exp->child->name, "ID")){
        struct ExtSListNode* vari = InExtList(exp->child->id_name);
        if (vari->inParam == 1)
            return c_assign(place, 0, vari->no);
        else
            return c_assign(place, 2, vari->no);
    }
    //array
    else if (exp->child->sibling != NULL&&!strcmp(exp->child->sibling->name, "LB")){
        //exp -> exp1 LB exp2 RB
            struct Node* exp1 = exp->child;
            struct Node* exp2 = exp1->sibling->sibling;

            int width = getTypeWidth(getExpType(exp));    //exp's width

            //PART1
            struct InterCode* part1 = NULL;     //calculate offset byte
            int t1 = new_temp();    //record offset byte

            if (!strcmp(exp2->child->name, "ID")&&exp2->child->sibling == NULL){
                struct ExtSListNode* vari = InExtList(exp2->child->id_name);
                part1 = c_operation(t1, 0, vari->no, "*", 1, width);
            }
            else{
                int t3 = new_temp();
                struct InterCode* code1 = translate_Exp(exp2, t3);
                struct InterCode* code2 = c_operation(t1, 0, t3, "*", 1, width);
                part1 = bindCode(code1, code2);
            }

            //PART2
            struct InterCode* part2 = NULL;     //calculate exp0's addr

            if (!strcmp(exp1->child->name, "ID")&&exp1->child->sibling == NULL){
                struct ExtSListNode* vari = InExtList(exp1->child->id_name);
                if (vari->inParam == 1)
                    part2 = c_operation(place, 0, vari->no, "+", 0, t1);
                else{
                    int tmp = new_temp();
                    struct InterCode *code1 = c_assign(tmp, 3, vari->no);
                    struct InterCode* code2 = c_operation(place, 0, tmp, "+", 0, t1);
                    part2 = bindCode(code1, code2);
                }
            }
            else{
                int t3 = new_temp();
                struct InterCode* code1 = translate_addr(exp1, t3);    //exp1's addr
                struct InterCode* code2 = c_operation(place, 0, t3, "+", 0, t1);    //exp0's addr
                part2 = bindCode(code1, code2);
            }

            return bindCode(part1, part2);
    }
    //struct
    else if (exp->child->sibling != NULL&&!strcmp(exp->child->sibling->name, "DOT")){
        //exp -> exp1 DOT ID
        struct Node* exp1 = exp->child;
        struct Node* id = exp1->sibling->sibling;

        struct TypeNode* exp1_type = getExpType(exp1);
        int offset = 0;
        for (struct TypeNode* p = exp1_type; strcmp(p->name, id->id_name); p = p->next){
            if (p->type != 2)
                offset += 4;
            else
                offset += getTypeWidth(p->Type);
        }
        if (!strcmp(exp1->child->name, "ID")){
            struct ExtSListNode* vari = InExtList(exp1->child->id_name);
            if (vari->inParam == 1)
                return c_operation(place, 0 ,vari->no, "+", 1, offset);
            else{
                int tmp = new_temp();
                struct InterCode *code1 = c_assign(tmp, 2, vari->no);
                struct InterCode* code2 = c_operation(place, 0, tmp, "+", 1, offset);
                return bindCode(code1, code2);
            }
        }
        else{
            int t1 = new_temp();
            struct InterCode* code1 = translate_addr(exp1, t1);    //exp1's addr
            struct InterCode* code2 = c_operation(place, 0 ,t1, "+", 1, offset);    //exp's addr
            return bindCode(code1, code2);
        }
    }
}

struct InterCode* translate_Stmt(struct Node* stmt){
    #ifdef T_DIAG_MODE
    printf("translate_Stmt: BEGIN\n");
    #endif
    if (!strcmp(stmt->child->name, "Exp")){
        return translate_Exp(stmt->child, -2);//result.vari_no == -2则不打印
    }
    else if (!strcmp(stmt->child->name, "CompSt")){
        return translate_CompSt(stmt->child);
    }
    else if (!strcmp(stmt->child->name, "RETURN")){
        struct Node* exp = stmt->child->sibling;

        if (!strcmp(exp->child->name, "INT")){
            return c_return(1, exp->child->int_val);
        }
        else if (!strcmp(exp->child->name, "ID")){
            struct ExtSListNode* vari = InExtList(exp->child->id_name);
            return c_return(0, vari->no);
        }
        else{
        int t1 = new_temp();
        struct InterCode* code1 = translate_Exp(exp, t1);
        struct InterCode* code2 = c_return(0, t1);
        return bindCode(code1, code2);
        }
    }
    else if (!strcmp(stmt->child->name, "WHILE")){
        int label1 = new_label();
        int label2 = new_label();
        int label3 = new_label();

        struct InterCode* code1 = c_label(label1);
        struct InterCode* code2 = translate_Cond(stmt->child->sibling->sibling, label2, label3);
        struct InterCode* code3 = c_label(label2);
        struct InterCode* code4 = translate_Stmt(stmt->child->sibling->sibling->sibling->sibling);
        struct InterCode* code5 = c_goto(label1);
        struct InterCode* code6 = c_label(label3);

        struct InterCode* tmp1 = bindCode(code1, bindCode(code2, code3));
        struct InterCode* tmp2 = bindCode(code4, bindCode(code5, code6));
        return bindCode(tmp1, tmp2);
    }
    else if (stmt->child->sibling->sibling->sibling->sibling->sibling == NULL){
        //stmt -> IF LP exp RP stmt1
        int label1 = new_label();
        int label2 = new_label();

        struct InterCode* code1 = translate_Cond(stmt->child->sibling->sibling, label1, label2);
        struct InterCode* code2 = c_label(label1);
        struct InterCode* code3 = translate_Stmt(stmt->child->sibling->sibling->sibling->sibling);
        struct InterCode* code4 = c_label(label2);

        return bindCode(code1, bindCode(code2, bindCode(code3, code4)));
    }
    else{
        //stmt -> IF LP exp RP stmt1 ELSE stmt2 
        int label1 = new_label();
        int label2 = new_label();
        int label3 = new_label();

        struct Node* exp = stmt->child->sibling->sibling;
        struct Node* stmt1 = exp->sibling->sibling;
        struct Node* stmt2 = stmt1->sibling->sibling;

        struct InterCode* code1 = translate_Cond(exp, label1, label2);
        struct InterCode* code2 = c_label(label1);
        struct InterCode* code3 = translate_Stmt(stmt1);
        struct InterCode* code4 = c_goto(label3);
        struct InterCode* code5 = c_label(label2);
        struct InterCode* code6 = translate_Stmt(stmt2);
        struct InterCode* code7 = c_label(label3);

        struct InterCode* tmp1 = bindCode(code1, bindCode(code2, code3));
        struct InterCode* tmp2 = bindCode(code4, bindCode(code5, code6));
        return bindCode(tmp1, bindCode(tmp2, code7));
    }
}

struct InterCode* translate_Cond(struct Node* exp, int label_true, int label_false){
    #ifdef T_DIAG_MODE
    printf("translate_Cond: BEGIN\n");
    #endif
    if (!strcmp(exp->child->name, "INT")){
        if (exp->child->int_val == 0)
            return c_goto(label_false);
        else
            return c_goto(label_true);
    }
    else if (!strcmp(exp->child->name, "ID")){
        struct ExtSListNode* vari = InExtList(exp->child->id_name);
        struct InterCode* code1 = c_relop(0, vari->no, 5, 1, 0, label_false);
        struct InterCode* code2 = c_goto(label_true);
        return bindCode(code1, code2);
    }
    else if (!strcmp(exp->child->name, "NOT")){
        return translate_Cond(exp->child, label_false, label_true);
    }
    else if (exp->child->sibling != NULL&&!strcmp(exp->child->sibling->name, "AND")){
        int label1 = new_label();
        struct InterCode* code1 = translate_Cond(exp->child, label1, label_false);
        struct InterCode* code2 = c_label(label1);
        struct InterCode* code3 = translate_Cond(exp->child->sibling->sibling, label_true, label_false);
        return bindCode(code1, bindCode(code2, code3));
    }
    else if (exp->child->sibling != NULL&&!strcmp(exp->child->sibling->name, "OR")){
        int label1 = new_label();
        struct InterCode* code1 = translate_Cond(exp->child, label_true, label1);
        struct InterCode* code2 = c_label(label1);
        struct InterCode* code3 = translate_Cond(exp->child->sibling->sibling, label_true, label_false);
        return bindCode(code1, bindCode(code2, code3));
    }
    else if (exp->child->sibling != NULL&&!strcmp(exp->child->sibling->name, "RELOP")){
        struct Node* exp1 = exp->child;
        struct Node* exp2 = exp1->sibling->sibling;
        int op = exp->child->sibling->relop_no;

        if (!strcmp(exp1->child->name, "INT")){
            if (!strcmp(exp2->child->name, "INT")){
                struct InterCode* code1 = c_relop(1, exp1->child->int_val, op, 1, exp2->child->int_val, label_true);
                struct InterCode* code2 = c_goto(label_false);
                return bindCode(code1, code2);
            }
            else if (!strcmp(exp2->child->name, "ID")){
                struct ExtSListNode* vari = InExtList(exp2->child->id_name);
                struct InterCode* code1 = c_relop(1, exp1->child->int_val, op, 0, vari->no, label_true);
                struct InterCode* code2 = c_goto(label_false);
                return bindCode(code1, code2);
            }
            else{
                int t1 = new_temp();
                struct InterCode* code1 = translate_Exp(exp2, t1);
                struct InterCode* code2 = c_relop(1, exp1->child->int_val, op, 0, t1, label_true);
                struct InterCode* code3 = c_goto(label_false);
                return bindCode(code1, bindCode(code2, code3));
            }
        }
        else if (!strcmp(exp1->child->name, "ID")){
            if (!strcmp(exp2->child->name, "INT")){
                struct ExtSListNode* vari = InExtList(exp1->child->id_name);
                struct InterCode* code1 = c_relop(0, vari->no, op, 1, exp2->child->int_val, label_true);
                struct InterCode* code2 = c_goto(label_false);
                return bindCode(code1, code2);
            }
            else if (!strcmp(exp2->child->name, "ID")){
                struct ExtSListNode* vari1 = InExtList(exp1->child->id_name);
                struct ExtSListNode* vari2 = InExtList(exp2->child->id_name);
                struct InterCode* code1 = c_relop(0, vari1->no, op, 0, vari2->no, label_true);
                struct InterCode* code2 = c_goto(label_false);
                return bindCode(code1, code2);
            }
            else{
                int t1 = new_temp();
                struct ExtSListNode* vari = InExtList(exp1->child->id_name);
                struct InterCode* code1 = translate_Exp(exp2, t1);
                struct InterCode* code2 = c_relop(0, vari->no, op, 0, t1, label_true);
                struct InterCode* code3 = c_goto(label_false);
                return bindCode(code1, bindCode(code2, code3));
            }
        }
        else{
            if (!strcmp(exp2->child->name, "INT")){
                int t1 = new_temp();
                struct InterCode* code1 = translate_Exp(exp1, t1);
                struct InterCode* code2 = c_relop(0, t1, op, 1, exp2->child->int_val, label_true);
                struct InterCode* code3 = c_goto(label_false);
                return bindCode(code1, bindCode(code2, code3));
            }
            else if (!strcmp(exp2->child->name, "ID")){
                int t1 = new_temp();
                struct ExtSListNode* vari = InExtList(exp2->child->id_name);
                struct InterCode* code1 = translate_Exp(exp1, t1);
                struct InterCode* code2 = c_relop(0, t1, op, 0, vari->no, label_true);
                struct InterCode* code3 = c_goto(label_false);
                return bindCode(code1, bindCode(code2, code3));
            }
            else{
                int t1 = new_temp();
                int t2 = new_temp();
                
                struct InterCode* code1 = translate_Exp(exp1, t1);
                struct InterCode* code2 = translate_Exp(exp2, t2);
                struct InterCode* code3 = c_relop(0, t1, op, 0, t2, label_true);
                struct InterCode* code4 = c_goto(label_false);
                return bindCode(code1, bindCode(code2, bindCode(code3, code4)));
            }
        }
    }
}

struct InterCode* translate_Program(struct Node* program){
    return translate_ExtDefList(program->child);
}

struct InterCode* translate_ExtDefList(struct Node* extdeflist){
    if (extdeflist->child == NULL)
        return NULL;
    struct InterCode* code1 = translate_ExtDef(extdeflist->child);
    struct InterCode* code2 = translate_ExtDefList(extdeflist->child->sibling);
    return bindCode(code1, code2);
}

struct InterCode* translate_ExtDef(struct Node* extdef){
    #ifdef T_DIAG_MODE
    printf("translate_ExtDef: BEGIN\n");
    #endif
    /*if (!strcmp(extdef->child->sibling->name, "ExtDecList")){
        //struct and array declaration
        struct InterCode* ret = NULL;

        struct Node* extdeclist = extdef->child->sibling;
        struct Node* vardec = extdeclist->child;
        while (1){
            //get name
            struct Node* nont = vardec->child;
            while (strcmp(nont->name, "ID"))
                nont = nont->child;

            struct ExtSListNode* vari = InExtList(nont->id_name);
            
            if (vari->Type->type == 2){
                int size = getTypeWidth(vari->Type);
                struct InterCode* code = c_decmem(vari->no, size);
                ret = bindCode(ret, code);
            }
            
            if (vardec->sibling != NULL)
                vardec = vardec->sibling->sibling->child;
            else
                break;
        }
        return ret;
    }
    else */if (!strcmp(extdef->child->sibling->name, "FunDec")){
        struct Node* fundec = extdef->child->sibling;
        struct Node* varlist = fundec->child->sibling->sibling;

        struct InterCode* code1 = c_func(fundec->child->id_name);
        struct InterCode* code2 = NULL;
        if (!strcmp(varlist->name, "VarList"))
            code2 = translate_VarList(varlist);
        struct InterCode* code3 = translate_CompSt(fundec->sibling);

        return bindCode(code1, bindCode(code2, code3));
    }
    else
        return NULL;
}

struct InterCode* translate_ExtDecList(struct Node* extdeclist){
    //struct and array declaration
    return NULL;
}

struct InterCode* translate_VarList(struct Node* varlist){
    #ifdef T_DIAG_MODE
    printf("translate_VarList: BEGIN\n");
    #endif
    struct Node* vardec = varlist->child->child->sibling;

    //get name
    struct Node* nont = vardec->child;
    while (strcmp(nont->name, "ID"))
        nont = nont->child;

    //get variable number
    struct ExtSListNode* vari = InExtList(nont->id_name);
    struct InterCode* code1 = c_param(vari->no);

    if(varlist->child->sibling == NULL)
        return code1;
    else{
        struct InterCode* code2 = translate_VarList(varlist->child->sibling->sibling);
        return bindCode(code1, code2);
    }
}

struct InterCode* translate_CompSt(struct Node* compst){
    struct InterCode* code1 = translate_DefList(compst->child->sibling);
    struct InterCode* code2 = translate_StmtList(compst->child->sibling->sibling);
    return bindCode(code1, code2);
}

struct InterCode* translate_DefList(struct Node* deflist){
    if (deflist->child == NULL)
        return NULL;
    struct InterCode* code1 = translate_Def(deflist->child);
    struct InterCode* code2 = translate_DefList(deflist->child->sibling);
    return bindCode(code1, code2);
}

struct InterCode* translate_StmtList(struct Node* stmtlist){
    if (stmtlist->child == NULL)
        return NULL;
    struct InterCode* code1 = translate_Stmt(stmtlist->child);
    struct InterCode* code2 = translate_StmtList(stmtlist->child->sibling);
    return bindCode(code1, code2);
}

struct InterCode* translate_Def(struct Node* def){
    struct InterCode* ret = NULL;

    struct Node* declist = def->child->sibling;
    struct Node* dec = declist->child;
    while (1){
        struct Node* vardec = dec->child;
        while (1){
            if (vardec->sibling != NULL&&!strcmp(vardec->child->name, "ID")){
                struct ExtSListNode* vari = InExtList(vardec->child->id_name);
                struct Node* exp = vardec->sibling->sibling;
                if (!strcmp(exp->child->name, "INT")){
                    return c_assign(vari->no, 1, exp->child->int_val);
                }
                else if (!strcmp(exp->child->name, "ID")&&exp->child->sibling == NULL){
                    struct ExtSListNode* vari2 = InExtList(exp->child->id_name);
                    return c_assign(vari->no, 0, vari2->no);
                }
                else{
                    int t1 = new_temp();
                    struct InterCode* code1 = translate_Exp(exp, t1);
                    struct InterCode* code2 = c_assign(vari->no, 0, t1);
                    return bindCode(code1, code2);
                }
            }
            else{
                //get name
                struct Node* nont = vardec->child;
                while (strcmp(nont->name, "ID"))
                    nont = nont->child;

                struct ExtSListNode* vari = InExtList(nont->id_name);
                
                //struct and array declaration
                if (vari->Type->type == 2){
                    int size = getTypeWidth(vari->Type);
                    struct InterCode* code = c_decmem(vari->no, size);
                    ret = bindCode(ret, code);
                }
            }
            
            if (vardec->sibling != NULL)
                vardec = vardec->sibling->sibling->child;
            else
                break;
        }
        if (dec->sibling != NULL)
            dec = dec->sibling->sibling->child;
        else
            break;
    }
    return ret;
}