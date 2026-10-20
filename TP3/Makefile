CC = gcc
CFLAGS = -Wall
FLEX = flex

all: exo1

exo1: exo1.l
	$(FLEX) exo1.l
	$(CC) $(CFLAGS) lex.yy.c -o exo1

test: exo1
	./exo1 exo1.dat

clean:
	rm -f *.o lex.yy.c exo1
