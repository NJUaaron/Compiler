#ifndef _REG_H_
#define _REG_H_

#include "assembly.h"
#include "../../IR/include/ICList.h"
#include "../../semantic/include/symbolList.h"

#define MAX_VARINUM 1000    //最多的临时变量数，应不小于IR_no
#define REG_NUM 16          //可为普通变量分配的寄存器数

//寄存器表
struct RegNode{
    int no;         //寄存器编号
    int vari_no;    //储存的变量号，为-1则表示寄存器为空
};

//变量表
struct VariNode{
    enum
    {
        NONE,    //没有存放位置
        STATIC,  //存放在静态数据区
        STACK   //存放在栈
    } kind;
    int isTemp;     //是否是中间表示中生成的临时变量
    int reg_no;     //储存的寄存器编号，为-1表示当前没有储存在寄存器中
    int fp_offset;  //栈中相对于$fp的偏移
};

//函数中变量表（记录函数中所有的局部变量，不包括中间代码的临时变量）
struct locVNode{
    int no;
    int width;
    struct locVNode *next;
};

struct FunVNode{
    char func_name[20];
    struct locVNode *locV;  //函数中所有的变量,包括形参
    struct FunVNode *next;
};

void printFunVList();//打印函数中变量表

extern struct RegNode regList[REG_NUM];
extern struct VariNode variList[MAX_VARINUM];
extern struct FunVNode *funVList;
extern int diff; //$fp和$sp的差值，初始值为8（之间保存了返回地址和上一个$fp）

void init_regList();
void init_variList();

struct Assembly *clear_reg();//清空所有通用寄存器

struct locVNode *create_locV(int no, int width, struct locVNode *next); //locVNode的构造函数
int inlocV(int no, struct locVNode *head);  //检测no号变量是否在表中
void init_funVList(struct InterCode *head);

//寄存器分配
//!对右值变量的寄存器分配一定要放在左值前面！
struct Assembly *reg(int vari_no /*变量编号*/
                    ,int *reg_no /*返回要分配的寄存器号*/
                    ,struct InterCode *pc/*当前处理的中间代码位置*/
                    ,int rightValue/*是否是右值*/);

struct Assembly *SelectToFree(struct InterCode *pc, int* no); //选择需要释放的寄存器

char *transReg(int no); //根据编号翻译寄存器,如 $29 翻译成 $sp

#endif