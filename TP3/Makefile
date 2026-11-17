# Makefile racine : delegue a chaque exercice (exo1/, exo2/, exo3/)
DIRS = exo1 exo2 exo3

all test clean:
	@for d in $(DIRS); do echo "### $$d"; $(MAKE) -C $$d $@ || exit 1; done
