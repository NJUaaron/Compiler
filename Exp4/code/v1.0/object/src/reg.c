#include "reg.h"

struct RegNode regList[REG_NUM];
struct VariNode variList[MAX_VARINUM];
struct FunVNode *funVList = NULL;

int diff = 8;

void printFunVList(){
    for (struct FunVNode *p = funVList; p != NULL; p = p->next){
        printf("%s:", p->func_name);
        for (struct locVNode *q = p->locV; q != NULL; q = q->next){
            printf(" t%d", q->no);
        }
        printf("\n");
    }
}

void init_regList(){
    for (int i = 0; i <= 7; i++){   //$t0~$t7 函数调用者负责保存
        regList[i].no = 8 + i;
        regList[i].vari_no = -1;
    }
    for (int i = 8; i <= 15; i++){   //$s0~$s7 函数负责保存和恢复
        regList[i].no = 8 + i;
        regList[i].vari_no = -1;
    }
}

void init_variList(){
    for (int i = 0; i < IR_no; i++){
        variList[i].reg_no = -1;
        variList[i].isTemp = 1;
        variList[i].kind = NONE;
    }

    for (struct ExtSListNode *p = ExtSList; p != NULL; p = p->next){
        if (p->inFunc == 0)//全局变量，存放在静态区
            variList[p->no].kind = STATIC;
        variList[p->no].isTemp = 0;
    }
}

struct Assembly* clear_reg(){
    for (int i = 0; i < REG_NUM; i++)
        regList[i].vari_no = -1;
    for (int i = 0; i < IR_no; i++)
        variList[i].reg_no = -1;
    
    //save register
    struct Assembly *save = NULL;
    for (int i = 0; i < REG_NUM; i++){
        if (regList[i].vari_no != -1){
            int vari_no = regList[i].vari_no;   //需要保存的变量标号
            if (variList[vari_no].kind == STATIC)
            {
                //sw $reg, vi
                save = bind(save, create_as("sw", 2, 0, regList[i].no, 3, vari_no, 0, 0, NULL));
            }
            else if (variList[vari_no].kind == STACK){
                //sw $reg, fp_offset($fp)
                save = bind(save, create_as("sw", 3, 0, regList[i].no, 0, 30, 1, variList[vari_no].fp_offset, NULL));
            }
        }
    }
    return save;
}

struct locVNode *create_locV(int no, int width, struct locVNode *next){
    struct locVNode* node = (struct locVNode *)malloc(sizeof(struct locVNode));
    node->no = no;
    node->width = width;
    node->next = next;
    return node;
}

int inlocV(int no, struct locVNode *head){
    for (struct locVNode *p = head; p != NULL; p = p->next){
        if (p->no == no)
            return 1;   //在表中
    }
    return 0;           //不在表中
}

void init_funVList(struct InterCode* head){
    //找出每个函数中所有左值变量
    for (struct InterCode *p = head; p != NULL; p = p->next)
    {
        if (p->result.vari_no == -2)//没有左值
            continue;
        if (p->kind == FUNC)
        {
            struct FunVNode* node = (struct FunVNode *)malloc(sizeof(struct FunVNode));
            node->next = funVList;
            funVList = node;
            node->locV = NULL;
            strcpy(node->func_name, p->func_name);
        }
        else if (p->kind == PARAM){
            if (!inlocV(p->result.vari_no, funVList->locV)){
                funVList->locV = create_locV(p->result.vari_no, 4, funVList->locV);
            }
        }
        else if (p->kind == ASSIGN || p->kind == OPERATION){
            if (!inlocV(p->result.vari_no, funVList->locV)){
                funVList->locV = create_locV(p->result.vari_no, 4, funVList->locV);
            }
        }
        else if (p->kind == DEC){
            funVList->locV = create_locV(p->result.vari_no, p->arg1.val, funVList->locV);
        }
        else if (p->kind == CALL || p->kind == READ){
            if (p->result.vari_no != -1 && !inlocV(p->result.vari_no, funVList->locV))
                funVList->locV = create_locV(p->result.vari_no, 4, funVList->locV);
        }

        /*
        printf("row %d: ", cnt);
        if (funVList!= NULL){
            for (struct locVNode *q = funVList->locV; q != NULL; q = q->next){
                printf("%d,%d ", q->no, q->width);
            }
        }
        printf("\n");
        cnt++;
        */
    }
}

struct Assembly *reg(int vari_no, int *reg_no, struct InterCode *pc, int rightValue){
    if (variList[vari_no].reg_no == -1){    //不在寄存器中
        int freeReg = -1;   //空闲的寄存器编号
        for (int i = 0; i < REG_NUM; i++){
            if (regList[i].vari_no == -1){
                freeReg = regList[i].no;
                regList[i].vari_no = vari_no;
                break;
            }
        }

        struct Assembly *regSelect = NULL;  //寄存器调度
        struct Assembly *storeVal = NULL; //写回

        if (freeReg == -1){ //没有空闲的寄存器

            int *reg_no = (int *)malloc(4);
            regSelect = SelectToFree(pc, reg_no);
            freeReg = *reg_no;
            free(reg_no);

            if (freeReg < 0) //该寄存器中的值可以丢弃
                freeReg = -freeReg;
            else{           //寄存器中的值需要存回内存
                if (variList[regList[freeReg-8].vari_no].kind == STATIC){   //存放回静态区
                    //sw $freeReg, vi
                    storeVal = create_as("sw", 2, 0, freeReg, 3, regList[freeReg - 8].vari_no, 0, 0, NULL);
                }
                else if (variList[regList[freeReg-8].vari_no].kind == STACK){   //存放回栈
                    //sw $freeReg, fp_offset($fp)
                    storeVal = create_as("sw", 3, 0, freeReg, 0, 30, 1, variList[regList[freeReg-8].vari_no].fp_offset, NULL);
                }
                else{    //在栈中开辟新空间进行存放
                    //get width
                    int width = 4;
                    diff += width;
                    for (struct ExtSListNode *p = ExtSList; p != NULL; p = p->next)
                    {
                        if (p->no == regList[freeReg-8].vari_no){
                            width = getTypeWidth(p->Type);
                            break;
                        }
                    }

                    //addi $sp, -width
                    struct Assembly *code1 = create_as("addi", 2, 0, 29, 1, -width, 0, 0, NULL);
                    //sw $freeReg, -diff($fp)
                    struct Assembly *code2 = create_as("sw", 3, 0, freeReg, 0, 30, 1, -diff, NULL);

                    variList[regList[freeReg - 8].vari_no].fp_offset = -diff;      
                    storeVal = bind(code1, code2);
                }
            }

            regList[freeReg - 8].vari_no = vari_no;
        }

        *reg_no = freeReg;
        variList[vari_no].reg_no = freeReg;

        struct Assembly *loadVal = NULL;    //加载
        
        if (rightValue == 1){
            if (variList[vari_no].kind == STATIC){   //从静态区加载
                //sw $freeReg, vi
                loadVal = create_as("lw", 2, 0, freeReg, 3, vari_no, 0, 0, NULL);
            }
            else if (variList[vari_no].kind == STACK){   //从栈加载
                //sw $freeReg, fp_offset($fp)
                loadVal = create_as("lw", 3, 0, freeReg, 0, 30, 1, variList[vari_no].fp_offset, NULL);
            }
        }
        else
            ;   //不需要加载

        return bind(regSelect, bind(storeVal, loadVal));
        
    }
    else{   //在寄存器中
        *reg_no = variList[vari_no].reg_no;
        return NULL;
    }
}

struct Assembly *SelectToFree(struct InterCode *pc, int* no){
    *no = 8;
    return clear_reg();
}

char* transReg(int no){
    switch(no){
    case 0: return "$zero";
    case 2: return "$v0";
    case 3: return "$v1";
    case 4: return "$a0";
    case 5: return "$a1";
    case 6: return "$a2";
    case 7: return "$a3";
    case 8: return "$t0";
    case 9: return "$t1";
    case 10: return "$t2";
    case 11: return "$t3";
    case 12: return "$t4";
    case 13: return "$t5";
    case 14: return "$t6";
    case 15: return "$t7";
    case 16: return "$s0";
    case 17: return "$s1";
    case 18: return "$s2";
    case 19: return "$s3";
    case 20: return "$s4";
    case 21: return "$s5";
    case 22: return "$s6";
    case 23: return "$s7";
    case 24: return "$t8";
    case 25: return "$t9";
    case 29: return "$sp";
    case 30: return "$fp";
    case 31: return "$ra";
    default:
        return "$x";
    }
}