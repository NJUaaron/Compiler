#include <stdio.h>
#include "syntax.tab.h"
#include "../semantic/include/analyze.h"
#include "../IR/include/translate.h"
#include "../object/include/assembly.h"

extern void yyrestart (FILE *input_file);
extern int no_syn_error;
extern struct Node* syntaxTree;

int main(int argc, char** argv){
    if (argc <= 1)  return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    fclose(f);

    if (!no_syn_error)
        return 1;

    preload();
    analyze(syntaxTree);
    checkNoDef();
    if (errorList != NULL){
        errorSort();
        errorPrint();
        return 1;
    }

    struct InterCode *IC_head = translate_Program(syntaxTree);

    init_regList();
    init_variList();
    init_funVList(IC_head);

    #ifdef A_DIAG_MODE
    printf("====INTER CODE====\n");
    printInterCode(IC_head);

    printf("\n====INITIALIZING====\n");
    printFunVList();
    
    printf("\n====TRANSLATION====\n");
    #endif

    struct Assembly* object_head = translate_IC(IC_head);

    if (argc >= 3){
        f = fopen(argv[2], "w");
        writeAssembly(f, object_head);
        fclose(f);
        printf("Translation Done!\n");
    }
    else
        printInterCode(IC_head);
    return 0;
}