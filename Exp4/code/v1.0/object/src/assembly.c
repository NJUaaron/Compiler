#include "assembly.h"

struct Assembly *translate_IC(struct InterCode *head){
    struct Assembly *object_head = NULL;
    struct Assembly *object_tail = NULL;

    #ifdef A_DIAG_MODE
    int row = 0;
    #endif

    for (struct InterCode *ptr = head; ptr != NULL; ptr = ptr->next)
    {
        #ifdef A_DIAG_MODE
        printf("row%d start\n", row);
        row++;
        #endif

        struct Assembly *unit_head = translate_ICunit(ptr);

        if (unit_head == NULL)
            continue;

        struct Assembly *unit_tail = unit_head;
        while (unit_tail->next != NULL)
            unit_tail = unit_tail->next;

        if (object_head == NULL){
            object_head = unit_head;
            object_tail = unit_tail;
        }
        else{
            object_tail->next = unit_head;
            unit_head->prev = object_tail;
            object_tail = unit_tail;
        }
    }
    return object_head;
}

struct Assembly *create_as(char* op, int arg_num, int t1, int v1, int t2, int v2, int t3, int v3, char* func_name){
    struct Assembly* as = (struct Assembly *)malloc(sizeof(struct Assembly));
    strcpy(as->op, op);
    as->arg_num = arg_num;
    as->prev = NULL;
    as->next = NULL;
    as->arg[0].type = t1;
    as->arg[0].val = v1;
    as->arg[1].type = t2;
    as->arg[1].val = v2;
    as->arg[2].type = t3;
    as->arg[2].val = v3;
    if (func_name != NULL)
        strcpy(as->func_name, func_name);
    return as;
}

struct Assembly* bind(struct Assembly* a, struct Assembly* b){
    if (a == NULL)
        return b;
    else if (b == NULL)
        return a;

    struct Assembly* a_tail = a;
    while (a_tail->next != NULL)
        a_tail = a_tail->next;

    a_tail->next = b;
    b->prev = a_tail;
    return a;
}

struct Assembly *translate_ICunit(struct InterCode *ICnode){
    if (ICnode->result.vari_no == -2 && ICnode->kind != CALL)
        return NULL;

    //regAlloc是为变量分配寄存器时可能需要的汇编代码
    if (ICnode->kind == ASSIGN){
        struct Assembly *part1 = NULL;
        switch (ICnode->arg1.type)
        {
        case 0:
        { //register
            int *reg_no = (int*)malloc(4);
            struct Assembly *regAlloc1 = reg(ICnode->arg1.val, reg_no, ICnode, 1);
            
            int v2 = *reg_no;
            struct Assembly *regAlloc = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
            int v1 = *reg_no;
            free(reg_no);

            //move &v0, reg(x)
            struct Assembly *code = create_as("move", 2, 0, v1, 0, v2, 0, 0, NULL);

            part1 = bind(regAlloc1, code);
            break;
        }

        case 1:
        { //immediate
            int *reg_no = (int*)malloc(4);
            struct Assembly *regAlloc = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
            int v1 = *reg_no;
            free(reg_no);

            part1 = create_as("li", 2, 0, v1, 1, ICnode->arg1.val, 0, 0, NULL);
            break;
        }

        case 2:
        { //address
            int offset = variList[ICnode->arg1.val].fp_offset;

            int *reg_no = (int*)malloc(4);
            struct Assembly *regAlloc = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
            int v1 = *reg_no;
            free(reg_no);

            //addi $v0, $fp, offset
            part1 = create_as("addi", 3, 0, v1, 0, 30, 1, offset, NULL);
            break;
        }

        case 3:
        { //pointer
            int *reg_no = (int*)malloc(4);
            struct Assembly *regAlloc1 = reg(ICnode->arg1.val, reg_no, ICnode, 1);
            int v2 = *reg_no;
            struct Assembly *regAlloc = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
            int v1 = *reg_no;
            free(reg_no);

            //move &v0, reg(x)
            struct Assembly *code = create_as("lw", 3, 0, v1, 0, v2, 1, 0, NULL);
            
            part1 = bind(regAlloc1, code);
        }

        }
    }
    else if (ICnode->kind == OPERATION){
        int *reg_no = (int*)malloc(4);
        struct Assembly *regAlloc2 = reg(ICnode->arg1.val, reg_no, ICnode, 1);
        int v2 = *reg_no;
        free(reg_no);

        if (!strcmp(ICnode->op.type, "+")){
            if (ICnode->arg2.type == 0){
                int *reg_no = (int*)malloc(4);
                struct Assembly *regAlloc3 = reg(ICnode->arg2.val, reg_no, ICnode, 1);
                int v3 = *reg_no;

                struct Assembly *regAlloc1 = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
                int v1 = *reg_no;
                free(reg_no);

                struct Assembly *code = create_as("add", 3, 0, v1, 0, v2, 0, v3, NULL);
                return bind(regAlloc1, bind(regAlloc2, bind(regAlloc3, code)));
            }
            else if (ICnode->arg2.type == 1){
                int *reg_no = (int*)malloc(4);
                struct Assembly *regAlloc1 = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
                int v1 = *reg_no;
                free(reg_no);

                struct Assembly *code = create_as("add", 3, 0, v1, 0, v2, 1, ICnode->arg2.val, NULL);
                return bind(regAlloc1, bind(regAlloc2, code));
            }
        }
        else if (!strcmp(ICnode->op.type, "-")){
            if (ICnode->arg2.type == 0){
                int *reg_no = (int*)malloc(4);
                struct Assembly *regAlloc3 = reg(ICnode->arg2.val, reg_no, ICnode, 1);
                int v3 = *reg_no;

                struct Assembly *regAlloc1 = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
                int v1 = *reg_no;
                free(reg_no);

                struct Assembly *code = create_as("sub", 3, 0, v1, 0, v2, 0, v3, NULL);
                return bind(regAlloc1, bind(regAlloc2, bind(regAlloc3, code)));
            }
            else if (ICnode->arg2.type == 1){
                int *reg_no = (int*)malloc(4);
                struct Assembly *regAlloc1 = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
                int v1 = *reg_no;
                free(reg_no);

                struct Assembly *code = create_as("addi", 3, 0, v1, 0, v2, 1, -(ICnode->arg2.val), NULL);
                return bind(regAlloc1, bind(regAlloc2, code));
            }
        }
        else if (!strcmp(ICnode->op.type, "*")){
            int *reg_no = (int*)malloc(4);
            struct Assembly *regAlloc3 = reg(ICnode->arg2.val, reg_no, ICnode, 1);
            int v3 = *reg_no;

            struct Assembly *regAlloc1 = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
            int v1 = *reg_no;
            free(reg_no);

            struct Assembly *code = create_as("mul", 3, 0, v1, 0, v2, 0, v3, NULL);
            return bind(regAlloc1, bind(regAlloc2, bind(regAlloc3, code)));
        }
        else if (!strcmp(ICnode->op.type, "/")){    //x := y / z       
            int *reg_no = (int*)malloc(4);
            struct Assembly *regAlloc3 = reg(ICnode->arg2.val, reg_no, ICnode, 1);
            int v3 = *reg_no;

            struct Assembly *regAlloc1 = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
            int v1 = *reg_no;
            free(reg_no);

            //div $reg(y), $reg(z)
            struct Assembly *code1 = create_as("div", 2, 0, v2, 0, v3, 0, 0, NULL);

            //mflo $reg(x)
            struct Assembly *code2 = create_as("mflo", 1, 0, v1, 0, 0, 0, 0, NULL);

            struct Assembly *part1 = bind(regAlloc1, bind(regAlloc2, regAlloc3));
            struct Assembly *part2 = bind(code1, code2);
            return bind(part1, part2);
        }
        else
            return NULL;
    }

    else if (ICnode->kind == LABEL)
        return create_as("label", 1, 2, ICnode->result.vari_no, 0, 0, 0, 0, NULL);

    else if (ICnode->kind == _RETURN){   //RETURN x
        //PART0: 将寄存器中的所有全局变量存回静态数据区
        struct Assembly *part0 = NULL;
        for (int i = 0; i <= 15; i++){
            if (regList[i].vari_no == -1)
                continue;
            int vari_no = regList[i].vari_no;
            if (variList[vari_no].kind == STATIC){
                int reg_no = regList[i].no;
                //sw $no, vari_no
                part0 = bind(part0, create_as("sw", 2, 0, reg_no, 3, vari_no, 0, 0, NULL));
            }
        }

        //PART1: move $v0, reg(x)
        struct Assembly *part1 = NULL;
        switch (ICnode->arg1.type)
        {
        case 0:
        { //register
            int *reg_no = (int*)malloc(4);
            struct Assembly *regAlloc = reg(ICnode->arg1.val, reg_no, ICnode, 1);
            //move &v0, reg(x)
            struct Assembly *code = create_as("move", 2, 0, 2, 0, *reg_no, 0, 0, NULL);
            free(reg_no);

            part1 = bind(regAlloc, code);
            break;
        }

        case 1:
        { //immediate
            part1 = create_as("li", 2, 0, 2, 1, ICnode->arg1.val, 0, 0, NULL);
            break;
        }

        case 2:
        { //address
            int offset = variList[ICnode->arg1.val].fp_offset;
            //addi $v0, $fp, offset
            part1 = create_as("addi", 3, 0, 2, 0, 30, 1, offset, NULL);
            break;
        }

        case 3:
        { //pointer
            int *reg_no = (int*)malloc(4);
            struct Assembly *regAlloc = reg(ICnode->arg1.val, reg_no, ICnode, 1);
            //move &v0, reg(x)
            struct Assembly *code = create_as("lw", 3, 0, 2, 0, *reg_no, 1, 0, NULL);
            free(reg_no);
            part1 = bind(regAlloc, code);
        }
        }

        //PART2: jr $ra
        struct Assembly *part2 = create_as("jr", 1, 0, 31, 0, 0, 0, 0, NULL);

        return bind(part0, bind(part1, part2));
    }

    else if (ICnode->kind == GOTO)
        return create_as("j", 1, 2, ICnode->result.vari_no, 0, 0, 0, 0, NULL);

    else if (ICnode->kind == _RELOP){
        int v1 = 24;    //$t8
        int v2 = 25;    //$t9

        struct Assembly *part1 = NULL;
        if (ICnode->arg1.type == 1){
            // li $t8, imm
            part1 = create_as("li", 2, 0, v1, 1, ICnode->arg1.val, 0, 0, NULL);
        }
        else if (ICnode->arg1.type == 0){
            int *reg_no = (int *)malloc(4);
            part1 = reg(ICnode->arg1.val, reg_no, ICnode, 1);
            v1 = *reg_no;
            free(reg_no);
        }

        struct Assembly *part2 = NULL;
        if (ICnode->arg2.type == 1){
            // li $t9, imm
            part2 = create_as("li", 2, 0, v2, 1, ICnode->arg2.val, 0, 0, NULL);
        }
        else if (ICnode->arg2.type == 0){
            int *reg_no = (int *)malloc(4);
            part2 = reg(ICnode->arg2.val, reg_no, ICnode, 1);
            v2 = *reg_no;
            free(reg_no);
        }

        struct Assembly *code = NULL;
        switch (ICnode->relop_no)
        {
        case 1: code = create_as("bgt", 3, 0, v1, 0, v2, 2, ICnode->result.vari_no, NULL); break;
        case 2: code = create_as("blt", 3, 0, v1, 0, v2, 2, ICnode->result.vari_no, NULL); break;
        case 3: code = create_as("bge", 3, 0, v1, 0, v2, 2, ICnode->result.vari_no, NULL); break;
        case 4: code = create_as("ble", 3, 0, v1, 0, v2, 2, ICnode->result.vari_no, NULL); break;
        case 5: code = create_as("beq", 3, 0, v1, 0, v2, 2, ICnode->result.vari_no, NULL); break;
        case 6: code = create_as("bne", 3, 0, v1, 0, v2, 2, ICnode->result.vari_no, NULL); break;
        }

        return bind(part1, bind(part2, code));
    }
    else if (ICnode->kind == CALL){
        //addi $sp, $sp, -8
        struct Assembly *code1 = create_as("addi", 3, 0, 29, 0, 29, 1, -8, NULL);
        //sw $ra, 4($sp)
        struct Assembly *code2 = create_as("sw", 3, 0, 31, 0, 29, 1, 4, NULL);
        //sw $fp, 0($sp)
        struct Assembly *code3 = create_as("sw", 3, 0, 30, 0, 29, 1, 0, NULL);
        //save registers
        struct Assembly *save = clear_reg();
        //jal func
        struct Assembly *code5 = create_as("jal", 0, 0, 0, 0, 0, 0, 0, ICnode->func_name);
        //move $sp, $fp
        struct Assembly *code6 = create_as("move", 2, 0, 29, 0, 30, 0, 0, NULL);
        //lw $ra, -4($sp)
        struct Assembly *code7 = create_as("lw", 3, 0, 31, 0, 29, 1, -4, NULL);
        //lw $fp, -8($sp)
        struct Assembly *code8 = create_as("lw", 3, 0, 30, 0, 29, 1, -8, NULL);

        struct Assembly *code9 = NULL;
        if (ICnode->result.vari_no != -2){//有返回值
            int *reg_no = (int*)malloc(4);
            struct Assembly *regAlloc = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
            //move $reg(vari_no), $ra
            struct Assembly *mov = create_as("move", 2, 0, *reg_no, 0, 2, 0, 0, NULL);
            free(reg_no);

            code9 = bind(regAlloc, mov);
        }

        struct Assembly *part1 = bind(code1, bind(code2, bind(code3, save)));
        struct Assembly *part2 = bind(code5, bind(code6, bind(code7, bind(code8, code9))));
        return bind(part1, part2);
    }

    else if (ICnode->kind == READ){
        //addi $sp, $sp, -4
        struct Assembly *code1 = create_as("addi", 3, 0, 29, 0, 29, 1, -4, NULL);
        //sw $ra, 0($sp)
        struct Assembly *code2 = create_as("sw", 3, 0, 31, 0, 29, 1, 0, NULL);
        //jal read
        struct Assembly *code3 = create_as("jal", 0, 0, 0, 0, 0, 0, 0, "read");
        //lw $ra, 0($sp)
        struct Assembly *code4 = create_as("lw", 3, 0, 31, 0, 29, 1, 0, NULL);
        //addi $sp, $sp, 4
        struct Assembly *code5 = create_as("addi", 3, 0, 29, 0, 29, 1, 4, NULL);

        int *reg_no = (int*)malloc(4);
        struct Assembly *regAlloc = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
        //move $reg(vari_no), $ra
        struct Assembly *mov = create_as("move", 2, 0, *reg_no, 0, 2, 0, 0, NULL);
        free(reg_no);

        struct Assembly *part1 = bind(code1, bind(code2, bind(code3, code4)));
        struct Assembly *part2 = bind(code5, bind(regAlloc, mov));
        return bind(part1, part2);
    }

    else if (ICnode->kind == WRITE){
        struct Assembly *code0 = NULL;
        if (ICnode->arg1.type == 1){//立即数
            //li $a0, imm
            code0 = create_as("li", 2, 0, 4, 1, ICnode->arg1.val, 0, 0, NULL);
        }
        else{//寄存器
            int *reg_no = (int *)malloc(4);
            struct Assembly *regAlloc = reg(ICnode->arg1.val, reg_no, ICnode, 1);
            //move $a0, reg(vari_no)
            struct Assembly *mov = create_as("move", 2, 0, 4, 0, *reg_no, 0, 0, NULL);
            free(reg_no);

            code0 = bind(regAlloc, mov);
        }

        //addi $sp, $sp, -4
        struct Assembly *code1 = create_as("addi", 3, 0, 29, 0, 29, 1, -4, NULL);
        //sw $ra, 0($sp)
        struct Assembly *code2 = create_as("sw", 3, 0, 31, 0, 29, 1, 0, NULL);
        //jal write
        struct Assembly *code3 = create_as("jal", 0, 0, 0, 0, 0, 0, 0, "write");
        //lw $ra, 0($sp)
        struct Assembly *code4 = create_as("lw", 3, 0, 31, 0, 29, 1, 0, NULL);
        //addi $sp, $sp, 4
        struct Assembly *code5 = create_as("addi", 3, 0, 29, 0, 29, 1, 4, NULL);

        struct Assembly *part1 = bind(code0, bind(code1, code2));
        struct Assembly *part2 = bind(code3, bind(code4, code5));
        return bind(part1, part2);
    }

    else if (ICnode->kind == ARG){
        int rank = 0;   //当前是第几个实参
        for (struct InterCode *p = ICnode; p->kind == ARG; p = p->prev)
            rank++;

        struct Assembly *code = NULL;
        
        if (rank > 4){ //没有空闲的参数寄存器，只能存在栈中
            int width = 4;
            diff += width;

            //addi $sp, -width
            struct Assembly *code1 = create_as("addi", 2, 0, 29, 1, -width, 0, 0, NULL);
            struct Assembly *code2 = NULL;

            if (ICnode->arg1.type == 0)
            { //变量
                int *reg_no = (int*)malloc(4);
                struct Assembly *regAlloc = reg(ICnode->arg1.val, reg_no, ICnode, 1);

                //sw $reg_no, -diff($fp)
                struct Assembly *store = create_as("sw", 3, 0, *reg_no, 0, 30, 1, -diff, NULL);
                free(reg_no);
                code2 = bind(regAlloc, store);
            }
            else{   //立即数
                //li $v1, imm
                struct Assembly *rec = create_as("li", 2, 0, 3, 1, ICnode->arg1.val, 0, 0, NULL);//立即数暂存在$v1中
                //sw $v1, -diff($fp)
                struct Assembly *store = create_as("sw", 3, 0, 3, 0, 30, 1, -diff, NULL);
            }        

            code = bind(code1, code2);
        }
        else{   //存在空闲的参数寄存器中
            int freeReg = rank + 3;    //空闲的参数寄存器编号

            if (ICnode->arg1.type == 0){        //变量
                
                if (variList[ICnode->arg1.val].reg_no == -1){    
                    //不在寄存器中
                    if (variList[ICnode->arg1.val].kind == STATIC){   //从静态区加载
                        //lw $freeReg, vi
                        code = create_as("lw", 2, 0, freeReg, 3, ICnode->arg1.val, 0, 0, NULL);
                    }
                    else if (variList[ICnode->arg1.val].kind == STACK){   //从栈加载
                        //lw $freeReg, fp_offset($fp)
                        code = create_as("lw", 3, 0, freeReg, 0, 30, 1, variList[ICnode->arg1.val].fp_offset, NULL);
                    }
                    else
                        ;   //不需要加载
                }

                //在寄存器中
                else{                                           
                    //move $freeReg, $reg(val)
                    code = create_as("move", 2, 0, freeReg, 0, variList[ICnode->arg1.val].reg_no, 0, 0, NULL);
                }
                
            }
            else if (ICnode->arg1.type == 1)    //立即数
                //li $freeReg, imm
                code = create_as("li", 2, 0, freeReg, 1, ICnode->arg1.val, 0, 0, NULL);
        }
        return code;
    }

    else if (ICnode->kind == PARAM){
        int rank = 0;   //当前是第几个形参
        for (struct InterCode *p = ICnode; p->kind == PARAM; p = p->prev)
            rank++;

        if (rank <= 4){//保存在寄存器中
            //sw $reg, offset($fp)
            return create_as("sw", 3, 0, rank + 3, 0, 30, 1, variList[ICnode->result.vari_no].fp_offset, NULL);
        }
        else{//保存在栈中
            int offset = 0;
            for (struct InterCode *p = ICnode->next;; p = p->next){
                if (p->kind == PARAM)
                    offset += 4;
                else
                    break;
            }
            //lw $v1, offset($fp)
            struct Assembly *code1 = create_as("lw", 3, 0, 3, 0, 30, 1, offset, NULL);
            //sw $v1, offset($fp)
            struct Assembly *code2 = create_as("sw", 3, 0, 3, 0, 30, 1, variList[ICnode->result.vari_no].fp_offset, NULL);
            return bind(code1, code2);
        }
    }

    else if (ICnode->kind == WRITEMEM){   
        int *reg_no = (int*)malloc(4);

        struct Assembly *regAlloc2 = reg(ICnode->arg1.val, reg_no, ICnode, 1);
        int v2 = *reg_no;
        struct Assembly *regAlloc1 = reg(ICnode->result.vari_no, reg_no, ICnode, 0);
        int v1 = *reg_no;
        
        free(reg_no);

        //sw $v1, 0($v2)
        struct Assembly *code = create_as("sw", 3, 0, v1, 0, v2, 1, 0, NULL);

        return bind(regAlloc1, bind(regAlloc2, code));
    }

    else if (ICnode->kind == DEC)
        return NULL;

    else if (ICnode->kind == FUNC){
        clear_reg();

        //addi $fp, $sp, 8
        struct Assembly *code0 = create_as("func", 0, 0, 0, 0, 0, 0, 0, ICnode->func_name);
        struct Assembly *code1 = create_as("addi", 3, 0, 30, 0, 29, 1, 8, ICnode->func_name);

        diff = 8; //$fp和$sp的差初始化为默认值
        struct FunVNode *p = funVList;
        for (; p != NULL; p = p->next)
            if(!strcmp(p->func_name, ICnode->func_name))
                break;
        if (p == NULL){
            printf("Function is not found in FunVList!\n");
            exit(-1);
        }

        int offset = -diff; //变量在栈中与$fp的偏移
        for (struct locVNode *q = p->locV; q != NULL; q = q->next)
        {
            #ifdef SAVE_ALL
            if (variList[q->no].kind != STATIC)
            #elif
            if (variList[q->no].isTemp != 1 && variList[q->no].kind != STATIC)
            #endif
            {
                //局部变量
                offset -= q->width;
                variList[q->no].kind = STACK;
                variList[q->no].fp_offset = offset;
            }
        }

        //addi $sp, offset
        struct Assembly *code2 = NULL;
        if (offset != 0)
            code2 = create_as("addi", 3, 0, 29, 0, 29, 1, offset-diff, NULL);

        return bind(code0, bind(code1, code2));
    }
}

void writeAssembly(FILE *f, struct Assembly *head){
    fprintf(f, ".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n");
    //静态数据区
    for (int i = 0; i < IR_no; i++){
        if(variList[i].kind == STATIC){
            int width = 4;
            for (struct ExtSListNode *p = ExtSList; p != NULL; p = p->next){
                if (p->no == i){
                    width = getTypeWidth(p->Type);
                    break;
                }
            }
            fprintf(f, "t%d: .space %d\n", i, width);
        }
    }
    fprintf(f, ".globl main\n.text");
    //代码区
    //read
    fprintf(f, "\nread:\n  li $v0, 4\n  la $a0, _prompt\n  syscall\n  li $v0, 5\n  syscall\n  jr $ra\n");

    //write
    fprintf(f, "\nwrite:\n  li $v0, 1\n  syscall\n  li $v0, 4\n  la $a0, _ret\n  syscall\n  move $v0, $0\n  jr $ra\n");

    //翻译的汇编代码
    for (struct Assembly *p = head; p != NULL; p = p->next)
    {
        if (!strcmp(p->op, "func")){
            fprintf(f, "\n%s:\n", p->func_name);
        }
        else if (!strcmp(p->op, "jal")){
            fprintf(f, "  jal %s\n", p->func_name);
        }
        else if (!strcmp(p->op, "label")){
            fprintf(f, "label%d:\n", p->arg[0].val);
        }
        else if ((!strcmp(p->op, "lw") || !strcmp(p->op, "sw"))&&p->arg_num == 3){
            fprintf(f, "  %s %s, %d(%s)\n", p->op, transReg(p->arg[0].val), p->arg[2].val, transReg(p->arg[1].val));
        }
        else{
            fprintf(f, "  %s ", p->op);
            for (int i = 0; i < p->arg_num; i++){
                if (i != 0)
                    fprintf(f, ", ");
                switch (p->arg[i].type){
                case 0:
                    fprintf(f, "%s", transReg(p->arg[i].val));
                    break;
                case 1:
                    fprintf(f, "%d", p->arg[i].val);
                    break;
                case 2:
                    fprintf(f, "label%d", p->arg[i].val);
                    break;
                case 3:
                    fprintf(f, "t%d", p->arg[i].val);
                    break;
                }
            }
            fprintf(f, "\n");
        }
    }
}