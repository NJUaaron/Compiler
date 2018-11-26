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

    printInterCode(translate_Program(syntaxTree));
    
    return 0;
}