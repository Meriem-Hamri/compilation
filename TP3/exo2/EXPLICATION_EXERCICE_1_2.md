# TP3 — Exercice 1.2 : suppression des commentaires C

## 1. Énoncé

> Développez un analyseur supprimant les commentaires dans un programme source C, c.-à-d. les chaînes de caractères `/* ... */`, pouvant s'étendre sur plusieurs lignes.

Le titre de l'exercice est *« Utilisation de variables dans les actions »* : on doit donc utiliser des variables C dans les actions Flex (ici, des compteurs).

## 2. Pourquoi une simple expression régulière ne suffit pas

On pourrait écrire `"/*"[^*]*"*"+([^*/][^*]*"*"+)*"/"`, qui reconnaît un commentaire complet. Elle est correcte mais illisible. Flex propose mieux : les **conditions de démarrage** (*start conditions*), c'est-à-dire des états de l'analyseur.

- `INITIAL` : on est dans le code C normal ; on recopie tout (`ECHO`).
- `COMMENTAIRE` : on est à l'intérieur d'un `/* ... */` ; on ignore tout.

On déclare l'état avec `%x COMMENTAIRE` (le `x` signifie *exclusif* : quand il est actif, seules les règles préfixées par `<COMMENTAIRE>` s'appliquent). On passe d'un état à l'autre avec `BEGIN(...)`.

## 3. Le fichier `exo2.l`

### Section déclarations — les variables

```c
int nb_commentaires = 0;      /* commentaires supprimés */
int nb_lignes_supprimees = 0; /* sauts de ligne avalés dans des commentaires */
int ligne_debut = 0;          /* ligne où le commentaire courant a commencé */
int ligne_courante = 1;       /* numéro de ligne dans le source */
```

Ces variables sont modifiées par les actions : c'est l'objet de l'exercice. Elles servent à produire un bilan et un message d'erreur précis.

### Section règles

| Règle | Rôle |
| :--- | :--- |
| `\"([^"\\n]\|\.)*\"` | Chaîne C : recopiée telle quelle. Ainsi `"a /* b */"` n'est **pas** pris pour un commentaire. |
| `\'([^'\\n]\|\.)+\'` | Constante caractère, même raison. |
| `"/*"` | Début de commentaire : `BEGIN(COMMENTAIRE)` et mémorisation de la ligne. |
| `<COMMENTAIRE>"*/"` | Fin : retour à `INITIAL`, `nb_commentaires++`. Rien n'est écrit, le commentaire disparaît. |
| `<COMMENTAIRE>\n` | Saut de ligne dans le commentaire : on compte, sans l'afficher. |
| `<COMMENTAIRE>[^*\n]+` | Tout texte sans `*` : ignoré. |
| `<COMMENTAIRE>"*"` | Étoile qui n'est pas suivie de `/` : ignorée. |
| `<COMMENTAIRE><<EOF>>` | Fin de fichier dans un commentaire : erreur « commentaire ouvert ligne N jamais fermé ». |
| `\n` et `.` | Code normal : recopié (`ECHO`) en comptant les lignes. |

### Pourquoi `"*"` et `[^*\n]+` séparés ?

Pour `/* ** etoiles ** */` : la règle `"*/"` est plus longue (2 caractères) que `"*"` (1), donc d'après la règle du **plus long préfixe**, Flex choisit `"*/"` dès que `*` est suivi de `/`. Sinon `"*"` consomme l'étoile seule et on continue. Un commentaire comme `/***/` est donc bien traité.

### Section code C

Le `main` ouvre le fichier passé en argument (sinon lit l'entrée standard), appelle `yylex()`, puis affiche le bilan sur `stderr` pour ne pas polluer le texte nettoyé écrit sur `stdout`.

## 4. Fichiers fournis

- `exo2.l` : l'analyseur.
- `exo2.dat` : programme C de test (commentaires multilignes, en fin de ligne, avec étoiles, et un faux commentaire dans une chaîne).
- `resultat_exo2.txt` : sortie obtenue.

## 5. Compilation et exécution

```bash
make exo2
./exo2 exo2.dat            # texte nettoyé sur stdout, bilan sur stderr
./exo2 exo2.dat > propre.c # enregistrer uniquement le code nettoyé
```

Sans Makefile : `flex -o lex2.yy.c exo2.l && gcc lex2.yy.c -o exo2`
(la commande de l'énoncé `gcc lex.yy.c -ll` est remplacée par `%option noyywrap`, qui évite d'avoir besoin de la bibliothèque `-ll`).

## 6. Résultat

Entrée (`exo2.dat`) :

```c
/* Programme de test : en-tete
   sur plusieurs lignes */
#include <stdio.h>

int main(void) {
    int a = 1; /* commentaire en fin de ligne */
    char *s = "ceci /* n'est pas un commentaire */";
    /* ** etoiles ** et / slash */
    a = a / 2;   /* division */
    /*
     * bloc
     */
    printf("%s %d\n", s, a);
    return 0;
}
```

Sortie :

```c

#include <stdio.h>

int main(void) {
    int a = 1; 
    char *s = "ceci /* n'est pas un commentaire */";
    
    a = a / 2;   
    
    printf("%s %d\n", s, a);
    return 0;
}
```

Bilan (stderr) : `5 commentaire(s) supprime(s), 3 saut(s) de ligne dans les commentaires`.

Observations :

- Les 5 commentaires ont disparu ; la division `a / 2` est intacte (un `/` seul ne démarre pas un commentaire).
- Le texte dans la chaîne `"ceci /* ... */"` est conservé.
- Le saut de ligne qui suit un commentaire est conservé (les lignes ne contenant qu'un commentaire deviennent des lignes vides) ; ceux **à l'intérieur** d'un commentaire sont supprimés (3 : 1 pour l'en-tête, 2 pour le bloc).

## 7. Limites

- Les commentaires `/* */` ne s'imbriquent pas en C : le premier `*/` ferme le commentaire, ce que fait l'analyseur.
- Les commentaires `//` du C99 ne sont pas traités, car l'énoncé ne demande que `/* ... */`.
