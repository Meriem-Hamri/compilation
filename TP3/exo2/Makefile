all: exo2

exo2: exo2.l
	flex -o lex2.yy.c exo2.l
	gcc -Wall lex2.yy.c -o exo2

test: exo2
	./exo2 exo2.dat

clean:
	rm -f lex2.yy.c exo2 exo2.exe
