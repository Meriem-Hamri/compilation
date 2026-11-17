%option noyywrap
%option nounput noinput
%option case-insensitive
%{
#include <stdio.h>
/* Exercice I.3 a) : mots contenant les 5 voyelles a,e,i,o,u dans l'ordre */
int nb_acceptes = 0, nb_rejetes = 0;
%}

L [a-z]
MOT_VALIDE    {L}*a{L}*e{L}*i{L}*o{L}*u{L}*

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
