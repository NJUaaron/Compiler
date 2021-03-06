#include <stdio.h>
#include "syntax.tab.h"
#include "../semantic/include/analyze.h"
#include "../IR/include/translate.h"

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

    struct InterCode* IChead = translate_Program(syntaxTree);
    if (argc >= 3){
        f = fopen(argv[2], "w");
        writeInterCode(f, IChead);
        fclose(f);
        printf("Translation Done!\n");
    }
    else
        printInterCode(IChead);
    
    return 0;
}