%option yylineno

%{
    #include "syntax.tab.h"
    #include <memory.h>
    int yycolumn = 1;
    /*
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng + 1; \
        yycolumn += yyleng;
    */

    struct Node{
        int lineno;
        char* name;
        struct Node* child;
        struct Node* sibling;
        int node_type;  //0: terminal  1:nonterminal  2:nonterminal=>empty string
        union{
            char* id_name;    //ID
            int int_val;
            float float_val;
            int relop_no;   //1~6
        };
    };

    struct Node* createNode(char* name, int lineno, int type){
        struct Node* node = (struct Node*)malloc(sizeof(struct Node));
        node->name = name;
        node->lineno = lineno;
        node->node_type = type;
        if (!strcmp(node->name, "ID")||!strcmp(node->name, "TYPE")) {
            char* buf = (char*)malloc(33);
            strcpy(buf, yytext);
            node->id_name = buf;
        }
        return node;
    }

    struct Node* createRELOPNode(int lineno){
        struct Node* node = (struct Node*)malloc(sizeof(struct Node));
        node->name = "RELOP";
        node->lineno = lineno;
        if (!strcmp(yytext, ">"))
            node->relop_no = 1;
        else if (!strcmp(yytext, "<"))
            node->relop_no = 2;
        else if (!strcmp(yytext, ">="))
            node->relop_no = 3;
        else if (!strcmp(yytext, "<="))
            node->relop_no = 4;
        else if (!strcmp(yytext, "=="))
            node->relop_no = 5;
        else if (!strcmp(yytext, "!="))
            node->relop_no = 6;
        return node;
    }

    struct Node* createINTNode(char* name, int value, int lineno){
        struct Node* node = (struct Node*)malloc(sizeof(struct Node));
        node->name = name;
        node->lineno = lineno;
        node->int_val = value;
        return node;
    }

    struct Node* createFLOATNode(char* name, float value, int lineno){
        struct Node* node = (struct Node*)malloc(sizeof(struct Node));
        node->name = name;
        node->lineno = lineno;
        node->float_val = value;
        return node;
    }

    void addChild(struct Node* a, struct Node* b){
        a->child = b;

        //非终结节点的行号等于第一个子节点的行号
        if (a->node_type == 1)    
            a->lineno = b->lineno;
    }

    void addSibling(struct Node* a, struct Node* b){
        a->sibling = b;
    }

    void print(struct Node* root, int depth){
        if (root == NULL)
            return;
        
        //推导出空串的非终结符号不进行打印
        if (root->node_type != 2){
            for(int i = 0; i < depth; i++)    //retract
                printf("  ");
            printf("%s",root->name);

            if (!strcmp(root->name, "ID")||!strcmp(root->name, "TYPE"))
                printf(" : %s ",root->id_name);
            else if (!strcmp(root->name, "INT"))
                printf(" : %d", root->int_val);
            else if (!strcmp(root->name, "FLOAT"))
                printf(" : %f", root->float_val);
            else if (root->node_type == 1)    //非终结符号打印行号
                printf(" (%d)", root->lineno);
            printf("\n");
        }

        print(root->child, depth + 1);    //此处不能使用++depth或depth++
        print(root->sibling, depth);
    }

%}

delim    [ \t]
ws    {delim}+
letter    [a-zA-Z]
letter_    {letter}|_
digit    [0-9]
posdigit    [1-9]
octdigit    [0-7]
hexdigit    [0-9A-Fa-f]
id    {letter}({letter}|{digit})*
float_type1    {digit}+\.?
float_type2    {digit}*\.{digit}+



%%
{ws}    {/*do nothing*/}
"//".*    {/*do nothing*/}
"/*"(.|\n)*"*/"    {/*do nothing*/}
\n    {yycolumn = 1; }
";"   {yylval.n = createNode("SEMI", yylineno, 0); return SEMI; }
","    {yylval.n = createNode("COMMA", yylineno, 0); return COMMA; }
"="    {yylval.n = createNode("ASSIGNOP", yylineno, 0); return ASSIGNOP; }
>|<|>=|<=|==|!=    {yylval.n = createRELOPNode(yylineno); return RELOP; }
"+"    {yylval.n = createNode("PLUS", yylineno, 0); return PLUS; }
"-"    {yylval.n = createNode("MINUS", yylineno, 0); return MINUS; }
"*"    {yylval.n = createNode("STAR", yylineno, 0); return STAR; }
"/"    {yylval.n = createNode("DIV", yylineno, 0); return DIV; }
"&&"    {yylval.n = createNode("AND", yylineno, 0); return AND; }
"||"    {yylval.n = createNode("OR", yylineno, 0); return OR; }
"."    {yylval.n = createNode("DOT", yylineno, 0); return DOT; }
"!"    {yylval.n = createNode("NOT", yylineno, 0); return NOT; }
"int"    {yylval.n = createNode("TYPE", yylineno, 0); return TYPE; }
"float"    {yylval.n = createNode("TYPE", yylineno, 0); return TYPE; }
"("    {yylval.n = createNode("LP", yylineno, 0); return LP; }
")"    {yylval.n = createNode("RP", yylineno, 0); return RP; }
"["    {yylval.n = createNode("LB", yylineno, 0); return LB; }
"]"    {yylval.n = createNode("RB", yylineno, 0); return RB; }
"{"    {yylval.n = createNode("LC", yylineno, 0); return LC; }
"}"    {yylval.n = createNode("RC", yylineno, 0); return RC; }
"struct"    {yylval.n = createNode("STRUCT", yylineno, 0); return STRUCT; }
"return"    {yylval.n = createNode("RETURN", yylineno, 0); return RETURN; }
"if"    {yylval.n = createNode("IF", yylineno, 0); return IF; }
"else"    {yylval.n = createNode("ELSE", yylineno, 0); return ELSE; }
"while"    {yylval.n = createNode("WHILE", yylineno, 0); return WHILE; }


{posdigit}{digit}*|0    {
    yylval.n = createINTNode("INT", atoi(yytext), yylineno);
    return INT;
}

0{octdigit}+    {
    int value = 0;
    yytext++;
    while(*yytext != '\0'){
        value = value*8 + *yytext -'0';
        yytext++;
    }
    yylval.n = createINTNode("INT", value, yylineno);
    return INT;
}

0x{hexdigit}+    {
    int value = 0;
    yytext += 2;
    while(*yytext != '\0'){
        if (*yytext >= '0' && *yytext <= '9')
            value = value*16 + *yytext -'0';
        else if (*yytext >= 'a' && *yytext <= 'f')
            value = value*16 + *yytext -'a' + 10;
        else if (*yytext >= 'A' && *yytext <= 'F')
            value = value*16 + *yytext -'A' + 10;
        yytext++;
    }
    yylval.n = createINTNode("INT", value, yylineno);
    return INT;
}

({float_type1}|{float_type2})((e|E)[+-]?{digit}+)?    {yylval.n = createFLOATNode("FLOAT", strtof(yytext, NULL), yylineno); return FLOAT; }

{letter_}({letter_}|{digit})*    {yylval.n = createNode("ID", yylineno, 0); return ID; }

.    {printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); }

%%
