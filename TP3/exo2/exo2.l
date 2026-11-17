%option noyywrap
%option nounput noinput
%x COMMENTAIRE
%{
#include <stdio.h>
#include <stdlib.h>

/* Variables utilisees dans les actions (objet de l'exercice 1.2) */
int nb_commentaires = 0;   /* commentaires supprimes             */
int nb_lignes_supprimees = 0; /* lignes couvertes par les commentaires */
int ligne_debut = 0;       /* ligne ou le commentaire courant a commence */
int ligne_courante = 1;    /* numero de ligne dans le fichier source */
%}

%%

\"([^"\\\n]|\\.)*\"   { ECHO; /* chaine C : on ne touche pas a son contenu */ }
\'([^'\\\n]|\\.)+\'   { ECHO; /* constante caractere */ }

"/*"                  { BEGIN(COMMENTAIRE); ligne_debut = ligne_courante; }

<COMMENTAIRE>{
  "*/"                { BEGIN(INITIAL); nb_commentaires++; }
  \n                  { ligne_courante++; nb_lignes_supprimees++; }
  [^*\n]+             { /* ignorer le texte du commentaire */ }
  "*"                 { /* etoile isolee : ignoree */ }
  <<EOF>>             { fprintf(stderr, "Erreur : commentaire ouvert ligne %d jamais ferme\n", ligne_debut);
                        return 1; }
}

\n                    { ligne_courante++; ECHO; }
.                     { ECHO; }

%%

int main(int argc, char *argv[]) {
    int code;
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) { perror("Erreur ouverture fichier"); return 1; }
        yyin = f;
    }
    code = yylex();
    fprintf(stderr, "\n--- %d commentaire(s) supprime(s), %d saut(s) de ligne dans les commentaires ---\n",
            nb_commentaires, nb_lignes_supprimees);
    return code;
}
