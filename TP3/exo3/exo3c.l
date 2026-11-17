%option noyywrap
%option nounput noinput

%{
#include <stdio.h>
/* Exercice I.3 c) : mots sur {a,b} sans deux occurrences consecutives du meme caractere */
int nb_acceptes = 0, nb_rejetes = 0;
%}

MOT_VALIDE    a|b|(ab)+a?|(ba)+b?

%%

{MOT_VALIDE}      { printf("%-20s -> ACCEPTE", yytext); nb_acceptes++; }
[^ \t\r\n]+       { printf("%-20s -> REJETE (non reconnu)", yytext); nb_rejetes++; }
[ \t\r\n]         { ECHO; /* delimiteurs : simple echo */ }

%%

int main(int argc, char *argv[]) {
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) { perror("Erreur ouverture fichier"); return 1; }
        yyin = f;
    }
    yylex();
    printf("--- %d accepte(s), %d rejete(s) ---\n", nb_acceptes, nb_rejetes);
    return 0;
}
