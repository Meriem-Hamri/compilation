%option noyywrap
%option nounput noinput
%{
#include <stdio.h>
#include <stdlib.h>
%}

/* Definitions de macros pour les expressions regulieres */
CHIFFRE       [0-9]
LETTRE        [a-zA-Z]
IDF           {LETTRE}({LETTRE}|{CHIFFRE})*
ENTIER        {CHIFFRE}+
FLOTTANT      ({CHIFFRE}+\.{CHIFFRE}*|\.{CHIFFRE}+)([eE][+-]?{CHIFFRE}+)?|{CHIFFRE}+[eE][+-]?{CHIFFRE}+
ESPACES       [ \t]+
NEWLINE       \n

%%

    /* 1. Mots-cles (doivent etre AVANT l'identificateur pour avoir la priorite) */
"for"           { printf("[MOT_CLE: %s] ", yytext); }
"while"         { printf("[MOT_CLE: %s] ", yytext); }
"if"            { printf("[MOT_CLE: %s] ", yytext); }
"else"          { printf("[MOT_CLE: %s] ", yytext); }
"int"           { printf("[MOT_CLE: %s] ", yytext); }
"float"         { printf("[MOT_CLE: %s] ", yytext); }
"return"        { printf("[MOT_CLE: %s] ", yytext); }

    /* 2. Operateurs composes */
"++"            { printf("[OP_INC: %s] ", yytext); }
"--"            { printf("[OP_DEC: %s] ", yytext); }
"=="            { printf("[OP_EQ: %s] ", yytext); }
"!="            { printf("[OP_NEQ: %s] ", yytext); }
"<="            { printf("[OP_LE: %s] ", yytext); }
">="            { printf("[OP_GE: %s] ", yytext); }
"&&"            { printf("[OP_AND: %s] ", yytext); }
"||"            { printf("[OP_OR: %s] ", yytext); }

    /* 3. Operateurs simples et ponctuation */
"="             { printf("[AFFECT: %s] ", yytext); }
"+"             { printf("[OP_PLUS: %s] ", yytext); }
"-"             { printf("[OP_MOINS: %s] ", yytext); }
"*"             { printf("[OP_MULT: %s] ", yytext); }
"/"             { printf("[OP_DIV: %s] ", yytext); }
"<"             { printf("[OP_LT: %s] ", yytext); }
">"             { printf("[OP_GT: %s] ", yytext); }
";"             { printf("[PVIRG: %s] ", yytext); }
","             { printf("[VIRG: %s] ", yytext); }
"("             { printf("[PAR_OUVR: %s] ", yytext); }
")"             { printf("[PAR_FERM: %s] ", yytext); }
"{"             { printf("[ACCO_OUVR: %s]\n", yytext); }
"}"             { printf("[ACCO_FERM: %s]\n", yytext); }
"["             { printf("[CROCH_OUVR: %s] ", yytext); }
"]"             { printf("[CROCH_FERM: %s] ", yytext); }

    /* 4. Nombres reels (flottants) puis entiers */
{FLOTTANT}      { printf("[NB_REEL: %s] ", yytext); }
{ENTIER}        { printf("[NB_ENTIER: %s] ", yytext); }

    /* 5. Identificateurs */
{IDF}           { printf("[IDF: %s] ", yytext); }

    /* 6. Espaces et sauts de ligne */
{ESPACES}       { /* ignorer les espaces */ }
{NEWLINE}       { putchar('\n'); }

    /* 7. Regle attrape-tout (caractere non reconnu) */
.               { printf("[INCONNU: %c] ", yytext[0]); }

%%

int main(int argc, char *argv[]) {
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            perror("Erreur ouverture fichier");
            return 1;
        }
        yyin = f;
    }
    yylex();
    return 0;
}
