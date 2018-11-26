%{
    #include "lex.yy.h"
    #include "syntaxTree.h"

    extern void yyerror(char* msg);

    int no_syn_error = 1;
    struct Node* syntaxTree = NULL;
%}

%union {
    struct Node* n;
}

//Declare tokens
%token <n> INT FLOAT ID
%token <n> ASSIGNOP AND OR RELOP PLUS MINUS STAR DIV
%token <n> SEMI COMMA DOT NOT TYPE
%token <n> LP RP LB RB LC RC
%token <n> STRUCT RETURN IF ELSE WHILE
%type <n> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args
%type <n> error


//Priority
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT 
%left LP RP LB RB DOT
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%
Program : ExtDefList    {
    $$ = createNode("Program", -1, 1);
    addChild($$, $1);
    syntaxTree = $$;
}

ExtDefList : ExtDef ExtDefList   {
    $$ = createNode("ExtDefList", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
}
  |    {$$ = createNode("ExtDefList", -1, 2);}
ExtDef : Specifier ExtDecList SEMI   {
    $$ = createNode("ExtDef", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Specifier SEMI   {
    $$ = createNode("ExtDef", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
}
  | Specifier FunDec CompSt   {
    $$ = createNode("ExtDef", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Specifier FunDec SEMI   {
    $$ = createNode("ExtDef", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Specifier error SEMI
  | error SEMI
ExtDecList : VarDec   {
    $$ = createNode("ExtDecList", -1, 1);
    addChild($$, $1);
}
  | VarDec COMMA ExtDecList   {
    $$ = createNode("ExtDecList", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}


Specifier : TYPE   {
    $$ = createNode("Specifier", -1, 1);
    addChild($$, $1);
}
  | StructSpecifier   {
    $$ = createNode("Specifier", -1, 1);
    addChild($$, $1);
}
StructSpecifier : STRUCT OptTag LC DefList RC   {
    $$ = createNode("StructSpecifier", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
    addSibling($3, $4);
    addSibling($4, $5);
}
  | STRUCT Tag   {
    $$ = createNode("StructSpecifier", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
}
OptTag : ID   {
    $$ = createNode("OptTag", -1, 1);
    addChild($$, $1);
}
  |   {
    $$ = createNode("OptTag", -1, 2);
}
Tag : ID   {
    $$ = createNode("Tag", -1, 1);
    addChild($$, $1);
}


VarDec : ID   {
    $$ = createNode("VarDec", -1, 1);
    addChild($$, $1);
}
  | VarDec LB INT RB   {
    $$ = createNode("VarDec", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
    addSibling($3, $4);
}
FunDec : ID LP VarList RP   {
    $$ = createNode("FunDec", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
    addSibling($3, $4);
}
  | ID LP RP   {
    $$ = createNode("FunDec", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | error LP VarList RP
  | ID LP error RP
  | error LP RP

VarList : ParamDec COMMA VarList   {
    $$ = createNode("VarList", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | ParamDec   {
    $$ = createNode("VarList", -1, 1);
    addChild($$, $1);
}
ParamDec : Specifier VarDec   {
    $$ = createNode("ParamDec", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
}


CompSt : LC DefList StmtList RC   {
    $$ = createNode("CompSt", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
    addSibling($3, $4);
}
  | LC DefList error RC
StmtList : Stmt StmtList   {
    $$ = createNode("StmtList", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
}
  |   {
    $$ = createNode("StmtList", -1, 2);
}
Stmt : Exp SEMI   {
    $$ = createNode("Stmt", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
}
  | CompSt   {
    $$ = createNode("Stmt", -1, 1);
    addChild($$, $1);
}
  | RETURN Exp SEMI   {
    $$ = createNode("Stmt", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE  {
    $$ = createNode("Stmt", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
    addSibling($3, $4);
    addSibling($4, $5);
}
  | IF LP Exp RP Stmt ELSE Stmt   {
    $$ = createNode("Stmt", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
    addSibling($3, $4);
    addSibling($4, $5);
    addSibling($5, $6);
    addSibling($6, $7);
}
  | WHILE LP Exp RP Stmt   {
    $$ = createNode("Stmt", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
    addSibling($3, $4);
    addSibling($4, $5);
}
  | error SEMI
  | RETURN error SEMI
  | IF LP error RP Stmt
  | IF LP error RP Stmt ELSE Stmt
  | WHILE LP error RP Stmt


DefList : Def DefList   {
    $$ = createNode("DefList", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
}
  |   {
    $$ = createNode("DefList", -1, 2);
}
Def : Specifier DecList SEMI   {
    $$ = createNode("Def", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Specifier error SEMI
DecList : Dec   {
    $$ = createNode("DecList", -1, 1);
    addChild($$, $1);
}
  | Dec COMMA DecList   {
    $$ = createNode("DecList", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
Dec : VarDec   {
    $$ = createNode("Dec", -1, 1);
    addChild($$, $1);
}
  | VarDec ASSIGNOP Exp   {
    $$ = createNode("Dec", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}


Exp : Exp ASSIGNOP Exp   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Exp AND Exp   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Exp OR Exp   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Exp RELOP Exp   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Exp PLUS Exp   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Exp MINUS Exp   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Exp STAR Exp   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Exp DIV Exp   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | LP Exp RP   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | MINUS Exp   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
}
  | NOT Exp   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
}
  | ID LP Args RP   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
    addSibling($3, $4);
}
  | ID LP RP   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Exp LB Exp RB   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
    addSibling($3, $4);
}
  | Exp DOT ID   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | ID   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
}
  | INT   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
}
  | FLOAT   {
    $$ = createNode("Exp", -1, 1);
    addChild($$, $1);
}
Args : Exp COMMA Args   {
    $$ = createNode("Args", -1, 1);
    addChild($$, $1);
    addSibling($1, $2);
    addSibling($2, $3);
}
  | Exp   {
    $$ = createNode("Args", -1, 1);
    addChild($$, $1);
}



%%

void yyerror(char* msg) {
    no_syn_error = 0;
    fprintf(stderr, "Error type B at Line %d: %s\n", yylineno, msg);
}