all: tp4
tp4: tp4.c
	gcc -Wall -o tp4 tp4.c
test: tp4
	./tp4 test.txt > resultat.txt; cat resultat.txt
clean:
	rm -f tp4 tp4.exe resultat.txt
