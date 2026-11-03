CC = gcc
CFLAGS = -Wall
FLEX = flex

all: exo1 exo2

exo1: exo1.l
	$(FLEX) exo1.l
	$(CC) $(CFLAGS) lex.yy.c -o exo1

exo2: exo2.l
	$(FLEX) -o lex2.yy.c exo2.l
	$(CC) $(CFLAGS) lex2.yy.c -o exo2

test: exo1
	./exo1 exo1.dat

test2: exo2
	./exo2 exo2.dat

clean:
	rm -f *.o lex.yy.c lex2.yy.c exo1 exo2
