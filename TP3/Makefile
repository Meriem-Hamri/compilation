CC = gcc
CFLAGS = -Wall
FLEX = flex

all: exo1 exo2 exo3a exo3b exo3c exo3d

exo1: exo1.l
	$(FLEX) exo1.l
	$(CC) $(CFLAGS) lex.yy.c -o exo1

exo2: exo2.l
	$(FLEX) -o lex2.yy.c exo2.l
	$(CC) $(CFLAGS) lex2.yy.c -o exo2

exo3%: exo3%.l
	$(FLEX) -o lex3$*.yy.c exo3$*.l
	$(CC) $(CFLAGS) lex3$*.yy.c -o exo3$*

test: exo1
	./exo1 exo1.dat

test2: exo2
	./exo2 exo2.dat

test3: exo3a exo3b exo3c exo3d
	@for x in a b c d; do echo "== exo3$$x"; ./exo3$$x exo3$$x.dat; done

clean:
	rm -f *.o lex.yy.c lex2.yy.c lex3?.yy.c exo1 exo2 exo3a exo3b exo3c exo3d
