all: tp5
tp5: tp5.c
	gcc -Wall -o tp5 tp5.c
test: tp5
	./tp5 test.txt > resultat.txt; cat resultat.txt
clean:
	rm -f tp5 tp5.exe resultat.txt
