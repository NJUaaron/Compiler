OBJS = *.o

syntax : syntax.tab.c lex.yy.c syntaxTree.c main.c
	gcc -c syntax.tab.c lex.yy.c syntaxTree.c main.c

syntax.tab.c : syntax.y
	bison -d syntax.y
	
lex.yy.c : lexical.l
	flex --header-file=lex.yy.h lexical.l

clean:
	rm lex.yy.c lex.yy.h syntax.tab.c syntax.tab.h $(OBJS)