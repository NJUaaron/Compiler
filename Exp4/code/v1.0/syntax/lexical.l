%option yylineno

%{
    #include "syntaxTree.h"
    #include "syntax.tab.h"

    int yycolumn = 1;
    /*
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng + 1; \
        yycolumn += yyleng;
    */

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
>|<|>=|<=|==|!=    {yylval.n = createRELOPNode(yylineno, yytext); return RELOP; }
"+"    {yylval.n = createNode("PLUS", yylineno, 0); return PLUS; }
"-"    {yylval.n = createNode("MINUS", yylineno, 0); return MINUS; }
"*"    {yylval.n = createNode("STAR", yylineno, 0); return STAR; }
"/"    {yylval.n = createNode("DIV", yylineno, 0); return DIV; }
"&&"    {yylval.n = createNode("AND", yylineno, 0); return AND; }
"||"    {yylval.n = createNode("OR", yylineno, 0); return OR; }
"."    {yylval.n = createNode("DOT", yylineno, 0); return DOT; }
"!"    {yylval.n = createNode("NOT", yylineno, 0); return NOT; }
"int"    {yylval.n = createTYPENode(yylineno, yytext); return TYPE; }
"float"    {yylval.n = createTYPENode(yylineno, yytext); return TYPE; }
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

{letter_}({letter_}|{digit})*    {yylval.n = createIDNode(yylineno, yytext); return ID; }

.    {printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); }

%%
