#include "ICList.h"

struct InterCode* ICList = NULL;
int IR_no = 0;
int label_no = 0;

int new_temp(){
    IR_no++;
    return IR_no - 1;
}

int new_label(){
    label_no++;
    return label_no - 1;
}

struct InterCode* bindCode(struct InterCode* a, struct InterCode* b){
    if (a == NULL)
        return b;
    else if (b == NULL)
        return a;
    struct InterCode* a_tail = a;
    while (a_tail->next != NULL)
        a_tail = a_tail->next;
    a_tail->next = b;
    b->prev = a_tail;
    return a;
}

struct InterCode* c_assign(int res, int t1, int v1){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = ASSIGN;
    code->result.vari_no = res;
    code->arg1.type = t1;
    code->arg1.val = v1;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_operation(int res, int t1, int v1, char* op, int t2, int v2){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = OPERATION;
    code->result.vari_no = res;
    code->arg1.type = t1;
    code->arg1.val = v1;
    code->op.type = (char*)malloc(33);
    strcpy(code->op.type, op);
    code->arg2.type = t2;
    code->arg2.val = v2;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_label(int no){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = LABEL;
    code->result.vari_no = no;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_return(int t1, int v1){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = _RETURN;
    code->result.vari_no = 0;
    code->arg1.type = t1;
    code->arg1.val = v1;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_goto(int no){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = GOTO;
    code->result.vari_no = no;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_relop(int t1, int v1, int op, int t2, int v2, int label){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = _RELOP;
    code->result.vari_no = label;
    code->arg1.type = t1;
    code->arg1.val = v1;
    code->relop_no = op;
    code->arg2.type = t2;
    code->arg2.val = v2;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_call(int return_no, char* func){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = CALL;
    code->result.vari_no = return_no;
    code->func_name = (char*)malloc(33);
    strcpy(code->func_name, func);
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_read(int return_no){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = READ;
    code->result.vari_no = return_no;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_write(int t1, int v1){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = WRITE;
    code->result.vari_no = 0;
    code->arg1.type = t1;
    code->arg1.val = v1;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_arg(int t1, int v1){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = ARG;
    code->result.vari_no = 0;
    code->arg1.type = t1;
    code->arg1.val = v1;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_param(int no){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = PARAM;
    code->result.vari_no = no;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_writemem(int no, int t1, int v1){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = WRITEMEM;
    code->result.vari_no = no;
    code->arg1.type = t1;
    code->arg1.val = v1;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_decmem(int no, int size){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = DEC;
    code->result.vari_no = no;
    code->arg1.val = size;
    code->next = NULL;
    code->prev = NULL;
    return code;
}

struct InterCode* c_func(char* func){
    struct InterCode* code = (struct InterCode*)malloc(sizeof(struct InterCode));
    code->kind = FUNC;
    code->result.vari_no = 0;
    code->func_name = (char*)malloc(33);
    strcpy(code->func_name, func);
    code->next = NULL;
    code->prev = NULL;
    return code;
}

void printInterCode(struct InterCode* head){
    for (struct InterCode* ptr = head; ptr != NULL; ptr = ptr->next){
        if (ptr->result.vari_no == -1)
            continue;
        switch(ptr->kind){
        case ASSIGN:
            printf("t%d := ", ptr->result.vari_no);
            switch(ptr->arg1.type){
            case 0: printf("t%d", ptr->arg1.val); break;
            case 1: printf("#%d", ptr->arg1.val); break;
            case 2: printf("&t%d", ptr->arg1.val); break;
            case 3: printf("*t%d", ptr->arg1.val);
            }
            break;

        case OPERATION:
            printf("t%d := ", ptr->result.vari_no);
            switch(ptr->arg1.type){
            case 0: printf("t%d", ptr->arg1.val); break;
            case 1: printf("#%d", ptr->arg1.val); break;
            case 2: printf("&t%d", ptr->arg1.val); break;
            case 3: printf("*t%d", ptr->arg1.val);
            }
            printf(" %s ", ptr->op.type);
            switch(ptr->arg2.type){
            case 0: printf("t%d", ptr->arg2.val); break;
            case 1: printf("#%d", ptr->arg2.val); break;
            case 2: printf("&t%d", ptr->arg2.val); break;
            case 3: printf("*t%d", ptr->arg2.val);
            }
            break;

        case LABEL:
            printf("LABEL label%d :", ptr->result.vari_no);
            break;

        case _RETURN:
            if (ptr->arg1.type == 0)
                printf("RETURN t%d", ptr->arg1.val);
            else if (ptr->arg1.type == 1)
                printf("RETURN #%d", ptr->arg1.val);
            break;

        case GOTO:
            printf("GOTO label%d", ptr->result.vari_no);
            break;

        case _RELOP:
            if (ptr->arg1.type == 0)
                printf("IF t%d ", ptr->arg1.val);
            else if (ptr->arg1.type == 1)
                printf("IF #%d ", ptr->arg1.val);   
            switch(ptr->relop_no){
            case 1: printf(">"); break;
            case 2: printf("<"); break;
            case 3: printf(">="); break;
            case 4: printf("<="); break;
            case 5: printf("=="); break;
            case 6: printf("!=");
            }
            if (ptr->arg2.type == 0)
                printf(" t%d ", ptr->arg2.val);
            else if (ptr->arg2.type == 1)
                printf(" #%d ", ptr->arg2.val);
            printf("GOTO label%d", ptr->result.vari_no);
            break;
            
        case CALL:
            printf("t%d := CALL %s", ptr->result.vari_no, ptr->func_name);
            break;

        case READ:
            printf("READ t%d", ptr->result.vari_no);
            break;
            
        case WRITE:
            if (ptr->arg1.type == 0)
                printf("WRITE t%d", ptr->arg1.val);
            else if (ptr->arg1.type == 1)
                printf("WRITE #%d", ptr->arg1.val);
            else if (ptr->arg1.type == 2)
                printf("WRITE &t%d", ptr->arg1.val);
            break;
            
        case ARG:
            if (ptr->arg1.type == 0)
                printf("ARG t%d", ptr->arg1.val);
            else if (ptr->arg1.type == 1)
                printf("ARG #%d", ptr->arg1.val);
            else if (ptr->arg1.type == 2)
                printf("ARG &t%d", ptr->arg1.val);
            break;
        
        case PARAM:
            printf("PARAM t%d", ptr->result.vari_no);
            break;
           
        case WRITEMEM:
            if (ptr->arg1.type == 0)
                printf("*t%d := t%d", ptr->result.vari_no, ptr->arg1.val);
            else if (ptr->arg1.type == 1)
                printf("*t%d := #%d", ptr->result.vari_no, ptr->arg1.val);  
            break;
           
        case DEC:
            printf("DEC t%d %d", ptr->result.vari_no, ptr->arg1.val);
            break;

        case FUNC:
            printf("FUNCTION %s :", ptr->func_name);
            break;

        default:
            printf("Undefined Intermediate Code Kind!");
        }
        printf("\n");
    }
}


