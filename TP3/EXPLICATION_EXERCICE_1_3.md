# TP3 — Exercice I.3 : expressions régulières

## 1. Objectif

Écrire quatre analyseurs Flex, chacun reconnaissant un langage :

| Fichier | Langage | Test |
|---|---|---|
| `exo3a.l` | mots contenant les 5 voyelles `a e i o u` dans l'ordre | `exo3a.dat` |
| `exo3b.l` | mots sur {a,b} commençant ou finissant par `a` | `exo3b.dat` |
| `exo3c.l` | mots sur {a,b} sans deux caractères identiques consécutifs | `exo3c.dat` |
| `exo3d.l` | mots sur {a,b} sans la sous-chaîne `aba` | `exo3d.dat` |

Principe commun (un mot par ligne) :

```
{MOT_VALIDE}      -> ACCEPTE
[^ \t\r\n]+       -> REJETE   (règle « attrape-tout » : tout mot non reconnu)
[ \t\r\n]         -> ECHO     (délimiteurs recopiés tels quels)
```

Les deux règles `{MOT_VALIDE}` et `[^ \t\r\n]+` reconnaissent la même chaîne quand le mot est valide : c'est **la première règle** qui gagne. Quand le mot est invalide (ex. `abc` pour le langage b), la règle générale reconnaît un texte **plus long** que `{MOT_VALIDE}` (qui ne reconnaîtrait que `ab`), donc **la plus longue correspondance** l'emporte et le mot est rejeté en entier.

## 2. Les expressions régulières

### a) 5 voyelles dans l'ordre
```
L [a-z]
MOT_VALIDE  {L}*a{L}*e{L}*i{L}*o{L}*u{L}*
```
Avec `%option case-insensitive` (majuscules acceptées). Les voyelles doivent apparaître dans l'ordre, avec n'importe quelles lettres entre elles. `facetious` et `abstemious` sont acceptés ; `uoiea` et `aeio` sont rejetés ; un chiffre (`ae1iou`) fait rejeter le mot.

### b) commence ou finit par `a`
```
a[ab]*  |  [ab]*a
```
Le mot `a` est accepté (il commence et finit par `a`).

### c) pas deux caractères consécutifs identiques
Les mots acceptés sont ceux qui **alternent** `a` et `b` :
```
a | b | (ab)+a? | (ba)+b?
```

### d) pas de sous-chaîne `aba`
On construit l'automate à 3 états : **S0** (dernier caractère `b`, ou début), **A** (on vient de lire `a`), **AB** (on vient de lire `ab`). Depuis AB, un `a` mènerait à `aba` : transition interdite. Un `b` depuis AB ramène à S0.

- boucles sur S0 : `b` ou `a+bb`
- états finaux : S0, A, AB, d'où `(b|a+bb)*(a+b?)?`

Pour éviter une expression qui reconnaît le mot vide (Flex bouclerait), on l'écrit :
```
X = b | a+bb
MOT_VALIDE  ({X})+(a+b?)? | a+b?
```
Vérifications : `aba` et `abab` rejetés ; `abb`, `bab`, `abba`, `babbaab` acceptés.

## 3. Exécution

```bash
make test3          # compile les 4 analyseurs et lance les tests
# ou à la main :
flex -o lex3b.yy.c exo3b.l
gcc lex3b.yy.c -o exo3b
./exo3b exo3b.dat
```

Les résultats obtenus sont dans `resultat_exo3.txt` (tous conformes aux langages attendus).
