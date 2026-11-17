/*
 * TP4 - Implementation d'automates : analyseur lexical (AFD + switch/case + Echec)
 * Unites : si, alors, sinon, oprel, id, nb
 *
 * Chaque AFD est une fonction qui part de l'etat initial, lit des caracteres
 * et retient la longueur du plus long prefixe reconnu (dernier etat final).
 * Si l'AFD ne reconnait rien, Echec() remet le pointeur au debut du lexeme
 * et passe a l'AFD suivant. Si tous echouent : erreur lexicale.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { T_SI, T_ALORS, T_SINON, T_OPREL, T_NB, T_ID, T_FIN, T_ERR } Token;
static const char *NOM[] = { "SI", "ALORS", "SINON", "OPREL", "NB", "ID", "FIN", "ERREUR" };

static const char *buf;   /* tampon d'entree */
static int debut;         /* debut du lexeme courant */
static int avance;        /* pointeur de lecture (forward) */
static int automate;      /* numero de l'AFD courant */

static int lire(void) { return buf[avance] ? (unsigned char)buf[avance++] : (avance++, 0); }

/* Echec : retour au debut du lexeme et branchement sur l'AFD suivant */
static int Echec(void) {
    avance = debut;
    return ++automate;
}

static int fin_mot(int c) { return !(isalnum(c)); }   /* delimiteur apres un mot-cle */

/* --- AFD mot-cle generique (si 0-1-2, alors 3..8, sinon 9..14) --- */
static int afd_motcle(const char *mot) {
    int i;
    for (i = 0; mot[i]; i++)
        if (lire() != (unsigned char)mot[i]) return -1;
    if (!fin_mot((unsigned char)buf[avance])) return -1;   /* ex: "sinon" != "si"+"non" */
    return avance - debut;
}

/* --- AFD oprel : < <= <> > >= = --- */
static int afd_oprel(void) {
    int etat = 15, c, dernier = -1;
    for (;;) {
        c = lire();
        switch (etat) {
        case 15:
            if (c == '<') etat = 16;
            else if (c == '>') etat = 17;
            else if (c == '=') etat = 18;
            else return dernier;
            break;
        case 16:                         /* final : "<" */
            dernier = avance - 1 - debut;
            if (c == '=' || c == '>') etat = 18; else return dernier;
            break;
        case 17:                         /* final : ">" */
            dernier = avance - 1 - debut;
            if (c == '=') etat = 18; else return dernier;
            break;
        case 18:                         /* final */
            return avance - 1 - debut;
        }
    }
}

/* --- AFD id : lettre (lettre | chiffre)* --- */
static int afd_id(void) {
    int etat = 19, c, dernier = -1;
    for (;;) {
        c = lire();
        switch (etat) {
        case 19:
            if (isalpha(c)) etat = 20; else return dernier;
            break;
        case 20:                         /* final */
            dernier = avance - 1 - debut;
            if (!isalnum(c)) return dernier;
            break;
        }
    }
}

/* --- AFD nb : chiffre+ (. chiffre+)? (E (+|-)? chiffre+)? --- */
static int afd_nb(void) {
    int etat = 21, c, dernier = -1;
    for (;;) {
        c = lire();
        switch (etat) {
        case 21: if (isdigit(c)) etat = 22; else return dernier; break;
        case 22:                         /* final : partie entiere */
            dernier = avance - 1 - debut;
            if (isdigit(c)) etat = 22;
            else if (c == '.') etat = 23;
            else if (c == 'E') etat = 25;
            else return dernier;
            break;
        case 23: if (isdigit(c)) etat = 24; else return dernier; break;
        case 24:                         /* final : partie decimale */
            dernier = avance - 1 - debut;
            if (isdigit(c)) etat = 24;
            else if (c == 'E') etat = 25;
            else return dernier;
            break;
        case 25:
            if (c == '+' || c == '-') etat = 26;
            else if (isdigit(c)) etat = 27;
            else return dernier;
            break;
        case 26: if (isdigit(c)) etat = 27; else return dernier; break;
        case 27:                         /* final : exposant */
            dernier = avance - 1 - debut;
            if (!isdigit(c)) return dernier;
            break;
        }
    }
}

/* Retourne le prochain lexeme ; remplit lexeme[] */
static Token prochain(char *lexeme, int *pos_debut) {
    while (buf[avance] && isspace((unsigned char)buf[avance])) avance++;
    debut = avance;
    *pos_debut = debut;
    if (!buf[avance]) { lexeme[0] = 0; return T_FIN; }

    for (automate = 0; automate <= 5; ) {
        int len = -1;
        avance = debut;
        switch (automate) {
        case 0: len = afd_motcle("si");    break;
        case 1: len = afd_motcle("alors"); break;
        case 2: len = afd_motcle("sinon"); break;
        case 3: len = afd_oprel();         break;
        case 4: len = afd_nb();            break;
        case 5: len = afd_id();            break;
        }
        if (len > 0) {
            static const Token t[] = { T_SI, T_ALORS, T_SINON, T_OPREL, T_NB, T_ID };
            memcpy(lexeme, buf + debut, len); lexeme[len] = 0;
            avance = debut + len;
            return t[automate];
        }
        Echec();                         /* essaie l'AFD suivant */
    }
    /* tous les AFD ont echoue : erreur lexicale, on saute 1 caractere */
    lexeme[0] = buf[debut]; lexeme[1] = 0;
    avance = debut + 1;
    return T_ERR;
}

int main(int argc, char **argv) {
    char ligne[1024], lexeme[1024];
    FILE *f = argc > 1 ? fopen(argv[1], "r") : stdin;
    int no = 0;
    if (!f) { perror(argv[1]); return 1; }
    while (fgets(ligne, sizeof ligne, f)) {
        Token t; int col;
        no++;
        ligne[strcspn(ligne, "\r\n")] = 0;
        buf = ligne; avance = 0;
        printf("Ligne %d : %s\n", no, ligne);
        while ((t = prochain(lexeme, &col)) != T_FIN) {
            if (t == T_ERR) printf("   ERREUR lexicale : caractere '%s' (col %d)\n", lexeme, col + 1);
            else            printf("   %-6s <%s>\n", NOM[t], lexeme);
        }
    }
    if (f != stdin) fclose(f);
    return 0;
}
