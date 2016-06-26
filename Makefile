new: new.c

jude: jude.lex main.c jude.h jude.y
	bison -d jude.y
	flex jude.lex
	gcc -g lex.yy.c main.c jude.tab.c -lfl -o $@

clean:
	rm -f lex.yy.c jude.tab.h jude jude.tab.c
