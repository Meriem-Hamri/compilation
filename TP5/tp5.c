/*
 * TP5 - Simulation d'un analyseur LL(1) pour G' avec recuperation en mode panique
 *
 * Grammaire G' (apres elimination de la recursivite a gauche) :
 *  1) R  -> T R'      2) R' -> + T R'   3) R' -> eps
 *  4) T  -> F T'      5) T' -> F T'     6) T' -> eps
 *  7) F  -> P F'      8) F' -> * F'     9) F' -> eps
 * 10) P  -> ( R )    11) P  -> a       12) P  -> b
 *
 * La table M[A,a] est construite au demarrage a partir de Premier/Suivant.
 * Mode panique (cours) : M[A,a] vide -> on saute 'a' ; si 'a' est dans
 * Suivant(A) (symbole de synchronisation) -> on depile A.
 * Terminal attendu different du terminal lu -> on depile le terminal (message).
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

enum { TA, TB, TPLUS, TSTAR, TLP, TRP, TEND, NT_ };         /* terminaux 0..6 */
enum { R = 10, R1, T, T1, F, F1, P };                       /* non-terminaux  */
static const char *TNOM[] = { "a", "b", "+", "*", "(", ")", "$" };
static const char *NNOM[] = { "R", "R'", "T", "T'", "F", "F'", "P" };
#define M(x) (1 << (x))

typedef struct { int g, d[3], n; unsigned first; } Regle;   /* first : sans eps */
static const Regle REG[13] = {
    {0},
    { R,  {T, R1},        2, M(TA)|M(TB)|M(TLP) },
    { R1, {TPLUS, T, R1}, 3, M(TPLUS) },
    { R1, {0},            0, 0 },
    { T,  {F, T1},        2, M(TA)|M(TB)|M(TLP) },
    { T1, {F, T1},        2, M(TA)|M(TB)|M(TLP) },
    { T1, {0},            0, 0 },
    { F,  {P, F1},        2, M(TA)|M(TB)|M(TLP) },
    { F1, {TSTAR, F1},    2, M(TSTAR) },
    { F1, {0},            0, 0 },
    { P,  {TLP, R, TRP},  3, M(TLP) },
    { P,  {TA},           1, M(TA) },
    { P,  {TB},           1, M(TB) },
};
/* Suivant(), indexe par non-terminal - 10 */
static const unsigned SUIVANT[7] = {
    M(TRP)|M(TEND),                                            /* R  */
    M(TRP)|M(TEND),                                            /* R' */
    M(TPLUS)|M(TRP)|M(TEND),                                   /* T  */
    M(TPLUS)|M(TRP)|M(TEND),                                   /* T' */
    M(TA)|M(TB)|M(TLP)|M(TPLUS)|M(TRP)|M(TEND),                /* F  */
    M(TA)|M(TB)|M(TLP)|M(TPLUS)|M(TRP)|M(TEND),                /* F' */
    M(TSTAR)|M(TA)|M(TB)|M(TLP)|M(TPLUS)|M(TRP)|M(TEND),       /* P  */
};
static int TAB[7][7];          /* TAB[nt-10][terminal] = numero de regle (0 = vide) */

static void construire_table(void) {
    int r, a, conflits = 0;
    for (r = 1; r <= 12; r++) {
        unsigned ens = REG[r].first;
        if (REG[r].n == 0) ens = SUIVANT[REG[r].g - 10];       /* regle epsilon */
        for (a = 0; a < 7; a++)
            if (ens & M(a)) {
                if (TAB[REG[r].g - 10][a]) conflits++;
                TAB[REG[r].g - 10][a] = r;
            }
    }
    if (conflits) fprintf(stderr, "ATTENTION : %d conflits, G' n'est pas LL(1)\n", conflits);
}

static void afficher_table(void) {
    int i, a;
    printf("Table d'analyse LL(1) :\n      ");
    for (a = 0; a < 7; a++) printf("%4s", TNOM[a]);
    for (i = 0; i < 7; i++) {
        printf("\n %-4s ", NNOM[i]);
        for (a = 0; a < 7; a++) { if (TAB[i][a]) printf("%4d", TAB[i][a]); else printf("   ."); }
    }
    printf("\n\n");
}

static const char *s; static int pos, lexerr;
/* prochain terminal ; -1 renvoye pour caractere inconnu (signale puis saute) */
static int lex(void) {
    for (;;) {
        char c;
        while (isspace((unsigned char)s[pos])) pos++;
        c = s[pos++];
        switch (c) {
        case 'a': return TA;   case 'b': return TB;
        case '+': return TPLUS; case '*': return TSTAR;
        case '(': return TLP;  case ')': return TRP;
        case 0:   pos--; return TEND;
        default:  lexerr++; printf("   [erreur lexicale] caractere '%c' ignore (col %d)\n", c, pos);
        }
    }
}

static void regle_str(int r, char *o) {
    int i; o += sprintf(o, "%s ->", NNOM[REG[r].g - 10]);
    if (!REG[r].n) sprintf(o, " eps");
    for (i = 0; i < REG[r].n; i++) {
        int x = REG[r].d[i];
        o += sprintf(o, " %s", x >= 10 ? NNOM[x - 10] : TNOM[x]);
    }
}

static int analyser(const char *mot) {
    int pile[256], sp = 0, nerr = 0, a;
    s = mot; pos = 0; lexerr = 0;
    pile[sp++] = TEND; pile[sp++] = R;
    a = lex();
    while (sp > 0) {
        int X = pile[sp - 1];
        if (X >= 10) {                                          /* non-terminal */
            int r = TAB[X - 10][a];
            if (r) {
                char buf[64]; regle_str(r, buf);
                printf("   %-14s lu '%s'\n", buf, TNOM[a]);
                sp--;
                for (int i = REG[r].n - 1; i >= 0; i--) pile[sp++] = REG[r].d[i];
            } else {
                nerr++;
                printf("   [erreur syntaxique] '%s' inattendu avec %s au sommet\n", TNOM[a], NNOM[X - 10]);
                if (SUIVANT[X - 10] & M(a)) { printf("   [panique] '%s' in Suivant(%s) : on depile %s\n", TNOM[a], NNOM[X - 10], NNOM[X - 10]); sp--; }
                else { printf("   [panique] on saute '%s'\n", TNOM[a]); a = lex(); }
            }
        } else if (X == a) {                                    /* terminal OK */
            sp--;
            if (X != TEND) a = lex();
        } else if (X == TEND) {                                 /* entree restante apres l'axiome */
            nerr++;
            printf("   [erreur syntaxique] '%s' en trop apres la fin de l'expression\n   [panique] on saute '%s'\n", TNOM[a], TNOM[a]);
            a = lex();
        } else {                                                /* terminal attendu != lu */
            nerr++;
            printf("   [erreur syntaxique] '%s' attendu, '%s' lu : '%s' insere (depile)\n", TNOM[X], TNOM[a], TNOM[X]);
            sp--;
        }
    }
    return nerr + lexerr;
}

int main(int argc, char **argv) {
    char ligne[512]; int no = 0;
    FILE *f = argc > 1 ? fopen(argv[1], "r") : stdin;
    if (!f) { perror(argv[1]); return 1; }
    construire_table();
    afficher_table();
    while (fgets(ligne, sizeof ligne, f)) {
        ligne[strcspn(ligne, "\r\n")] = 0;
        if (!ligne[0]) continue;
        printf("=== Mot %d : \"%s\"\n", ++no, ligne);
        int e = analyser(ligne);
        printf("=> %s\n\n", e ? "REJETE (erreurs recuperees en mode panique)" : "ACCEPTE");
    }
    if (f != stdin) fclose(f);
    return 0;
}
