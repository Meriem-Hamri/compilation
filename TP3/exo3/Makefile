all: exo3a exo3b exo3c exo3d

exo3%: exo3%.l
	flex -o lex3$*.yy.c exo3$*.l
	gcc -Wall lex3$*.yy.c -o exo3$*

test: all
	@for x in a b c d; do echo "== exo3$$x"; ./exo3$$x exo3$$x.dat; done

clean:
	rm -f lex3?.yy.c exo3a exo3b exo3c exo3d *.exe
