# TP3 – Exercice I.2 : Questions de cours et réponses

Fichiers concernés : [exo2.l](exo2.l), [exo2.dat](exo2.dat), [resultat_exo2.txt](resultat_exo2.txt)

> **Énoncé :** Développez un analyseur supprimant les commentaires dans un programme source C, c.-à-d. les chaînes de caractères `/* ... */`, pouvant s'étendre sur plusieurs lignes.
> **Titre :** Utilisation de variables dans les actions.

---

## Niveau 1 : comprendre l'exercice

### Q1. Que doit faire l'analyseur de l'exercice 2 ?

C'est un **filtre**. Il recopie le programme C reçu en entrée **à l'identique**, en retirant seulement les commentaires `/* ... */`, même quand ils s'étendent sur plusieurs lignes. Il ne produit pas de liste de tokens comme dans l'exercice 1 : sa sortie est encore du code C.

### Q2. Pourquoi l'exercice s'intitule-t-il « utilisation de variables dans les actions » ?

Les actions Flex sont du **code C ordinaire**. Elles peuvent donc lire et modifier des variables. Ici, les variables servent à :
- **compter** les commentaires supprimés (`nb_commentaires`) et les sauts de ligne avalés (`nb_lignes_supprimees`) ;
- **suivre la position** dans le fichier (`ligne_courante`) ;
- **mémoriser** la ligne où le commentaire courant a commencé (`ligne_debut`), pour afficher un message d'erreur précis.

Ces variables gardent leur valeur d'une règle à l'autre. C'est ce qui permet de faire un bilan à la fin de l'analyse.

### Q3. Où sont déclarées ces variables, et pourquoi peut-on les utiliser à la fois dans les règles et dans `main()` ?

Elles sont déclarées dans la **section 1**, entre `%{` et `%}`. Flex recopie ce code tel quel **au début de `lex.yy.c`**. Ce sont donc des **variables globales** du fichier C, visibles depuis `yylex()` (qui contient les actions) comme depuis `main()` (section 3).

### Q4. À quoi sert `ECHO` ?

`ECHO` recopie le lexème reconnu (`yytext`) sur la sortie `yyout` (`stdout` par défaut). C'est l'équivalent de `fwrite(yytext, yyleng, 1, yyout)`.

Dans cet exercice, c'est l'action utilisée pour **tout ce qui n'est pas un commentaire**.

---

## Niveau 2 : les états (start conditions)

### Q5. Qu'est-ce qu'une condition de démarrage (start condition) ?

C'est un **état** de l'analyseur. Une règle préfixée par `<ETAT>` ne s'applique **que lorsque l'analyseur est dans cet état**.
- L'état par défaut s'appelle **`INITIAL`** (ou `0`).
- On change d'état dans une action avec **`BEGIN(ETAT)`**.

Ici, il y a deux états :

| État | Signification | Comportement |
|---|---|---|
| `INITIAL` | dans le code C normal | tout est recopié (`ECHO`) |
| `COMMENTAIRE` | entre `/*` et `*/` | rien n'est écrit |

```
          "/*"                       "*/"
INITIAL ────────► COMMENTAIRE ────────► INITIAL
 (ECHO)            (on ignore)          nb_commentaires++
```

### Q6. Quelle est la différence entre `%x` et `%s` ?

- **`%x` (exclusif)** : quand l'état est actif, **seules** les règles préfixées par `<ETAT>` s'appliquent.
- **`%s` (inclusif)** : les règles `<ETAT>` s'appliquent, **plus** toutes les règles sans préfixe.

### Q7. Pourquoi avoir choisi `%x` et non `%s` ?

Avec `%s`, les règles du code normal resteraient actives à l'intérieur du commentaire. Par exemple :
- `/* voir "fichier" */` : la règle des chaînes `\"...\"` reconnaîtrait `"fichier"` et l'**afficherait** (`ECHO`), alors qu'il fait partie du commentaire.
- La règle `"/*"` pourrait se déclencher de nouveau à l'intérieur d'un commentaire.

Avec `%x`, on est sûr que **rien** ne sort tant qu'on est dans le commentaire.

### Q8. Comment revient-on dans l'état normal ?

Avec `BEGIN(INITIAL)` (on peut aussi écrire `BEGIN 0`), dans l'action de la règle `<COMMENTAIRE>"*/"`.

---

## Niveau 3 : les règles en détail

### Q9. Pourquoi ne pas écrire simplement la règle `"/*".*"*/"  { }` ?

Elle pose deux problèmes :
1. **`.` ne reconnaît pas `\n`**. Un commentaire sur plusieurs lignes ne serait donc jamais reconnu.
2. **Plus longue correspondance** : sur la ligne `a = 1; /* x */ b = 2; /* y */`, `.*` irait jusqu'au **dernier** `*/` de la ligne. Le code `b = 2;` serait supprimé avec les commentaires.

Une expression régulière exacte existe : `"/*"([^*]|\*+[^*/])*\*+"/"`. Mais elle est difficile à lire et à vérifier, et elle ne permet pas de compter facilement les lignes. Les états donnent une solution plus simple et plus claire.

### Q10. Dans l'état `COMMENTAIRE`, pourquoi utiliser `[^*\n]+` et `"*"` plutôt qu'une simple règle `.` ?

- `[^*\n]+` avale **d'un coup** un long morceau de texte sans `*`, ce qui est plus efficace que de traiter un caractère à la fois.
- Elle **s'arrête juste avant chaque `*`**. Ainsi, quand on arrive sur `*/`, la règle `"*/"` (2 caractères) l'emporte sur `"*"` (1 caractère) grâce à la plus longue correspondance.
- `"*"` ignore une **étoile isolée** qui n'est pas suivie de `/` (comme dans `** etoiles **`).
- `\n` est traité à part pour **compter** les lignes.

Si on avait écrit `[^\n]+`, la règle avalerait aussi le `*/`, et le commentaire **ne se terminerait jamais**.

### Q11. Comment est traité `**/` (deux étoiles puis un slash) ?

Le premier `*` est reconnu par `"*"` et ignoré. Ensuite, `*/` est reconnu par `"*/"`, ce qui ferme le commentaire.

### Q12. Pourquoi faut-il une règle pour les chaînes de caractères `\"...\"` ?

Sans elle, la ligne
```c
char *s = "ceci /* n'est pas un commentaire */";
```
serait abîmée : le `/*` placé dans la chaîne serait pris pour un début de commentaire. Avec cette règle, Flex reconnaît **la chaîne entière d'un seul coup** dès le `"` d'ouverture, et la recopie avec `ECHO`. Le `/*` qu'elle contient n'est donc jamais examiné tout seul.

### Q13. Explique l'expression `\"([^"\\\n]|\\.)*\"`.

| Morceau | Sens |
|---|---|
| `\"` | le guillemet d'ouverture |
| `[^"\\\n]` | tout caractère sauf `"`, `\` et un saut de ligne |
| `\\.` | une séquence d'échappement : `\` suivi de n'importe quel caractère (`\n`, `\"`, `\\`…) |
| `( ... \| ... )*` | une suite de ces éléments, éventuellement vide |
| `\"` | le guillemet de fermeture |

Grâce à `\\.`, une chaîne comme `"il dit \"bonjour\""` est reconnue en entier : le `\"` ne ferme pas la chaîne.

### Q14. Pourquoi y a-t-il aussi une règle pour les constantes caractères `'...'` ?

Elle sert à éviter des pièges comme :
- `'"'` : sans cette règle, le `"` serait pris pour le **début d'une chaîne** ;
- `'/'` suivi de `'*'` : ce sont des caractères, pas un commentaire.

### Q15. La division `a = a / 2;` est-elle conservée ? Pourquoi ?

Oui. La règle `"/*"` exige **deux caractères** : `/` puis `*`. Un `/` suivi d'un espace ne correspond pas à cette règle. Il est reconnu par la règle `.` et recopié avec `ECHO`.

---

## Niveau 4 : fin de fichier et erreurs

### Q16. À quoi sert la règle `<COMMENTAIRE><<EOF>>` ?

`<<EOF>>` est un motif spécial qui correspond à la **fin du fichier**. Préfixé par `<COMMENTAIRE>`, il se déclenche si le fichier se termine **alors qu'un commentaire est encore ouvert**. On affiche alors :
```
Erreur : commentaire ouvert ligne 12 jamais ferme
```
Le numéro de ligne vient de la variable `ligne_debut`, mémorisée au moment du `/*`. C'est un bon exemple de l'utilité des variables.

### Q17. Pourquoi cette action se termine-t-elle par `return 1;` ?

Une action `<<EOF>>` **doit arrêter l'analyse**, avec `return` ou `yyterminate()`. Sinon, `yylex()` continuerait à tourner sur la fin de fichier. Le `1` renvoyé par `yylex()` est récupéré dans `main()` et devient le **code de sortie** du programme, ce qui signale une erreur.

### Q18. Que se passerait-il sans cette règle `<<EOF>>` ?

Flex appliquerait son comportement par défaut : arrêter l'analyse **en silence**. Tout le code situé après le `/*` non fermé disparaîtrait, sans aucun message d'erreur.

### Q19. Pourquoi le bilan `--- 5 commentaire(s) ... ---` n'apparaît-il pas dans `resultat_exo2.txt` ?

Le bilan est affiché avec `fprintf(stderr, ...)`, sur la **sortie d'erreur**. La redirection `> resultat_exo2.txt` ne capture que la **sortie standard** (`stdout`). Le fichier ne contient donc que le code nettoyé, qui reste du C valide. Le bilan s'affiche seulement dans le terminal.

---

## Niveau 5 : exécution à la main

### Q20. Quelle est la sortie pour l'entrée `x = 1; /* a */ y = 2;` ?

```
x = 1;  y = 2;
```
Il y a **deux espaces** entre `1;` et `y` : celui qui précède le commentaire et celui qui le suit. Le commentaire est supprimé, mais pas les espaces autour de lui.

### Q21. Que deviennent les sauts de ligne d'un commentaire sur plusieurs lignes ?

Ils sont **supprimés** : la règle `<COMMENTAIRE>\n` ne fait pas d'`ECHO`, elle incrémente seulement les compteurs. Par exemple :
```c
    /*
     * bloc
     */
    printf(...);
```
devient :
```c
    
    printf(...);
```
Il reste seulement l'indentation qui précède `/*` et le saut de ligne qui suit `*/`. Le numéro de ligne de `printf` change donc dans le fichier produit.

### Q22. Combien de commentaires `exo2.dat` contient-il ? Que vaut chaque compteur à la fin ?

Il contient 5 commentaires :
1. l'en-tête sur 2 lignes (1 saut de ligne) ;
2. `/* commentaire en fin de ligne */` ;
3. `/* ** etoiles ** et / slash */` ;
4. `/* division */` ;
5. le bloc sur 3 lignes (2 sauts de ligne).

Le `/* ... */` placé **dans la chaîne** ne compte pas.

Le bilan affiché est donc :
```
--- 5 commentaire(s) supprime(s), 3 saut(s) de ligne dans les commentaires ---
```

### Q23. Que donne `/* a /* b */ c */` ? Est-ce correct ?

La sortie est ` c */`. Le **premier** `*/` ferme le commentaire. Le second `/*` n'est pas spécial, puisqu'on est déjà dans l'état `COMMENTAIRE`. La suite ` c */` est donc recopiée comme du code.

C'est **conforme au langage C** : les commentaires `/* */` ne s'imbriquent pas. Un compilateur C signalerait d'ailleurs une erreur de syntaxe sur ce ` c */`.

---

## Bonus : extensions possibles

### Q24. Comment supprimer aussi les commentaires `// ...` (C99/C++) ?

Il suffit d'ajouter une règle dans l'état `INITIAL` :
```lex
"//".*      { nb_commentaires++; /* on garde le \n qui suit */ }
```
Ici `.*` s'arrête avant le `\n`. Le saut de ligne est donc recopié normalement par la règle `\n`, ce qui préserve la numérotation des lignes. La règle des chaînes protège déjà `"http://..."`.

### Q25. Comment conserver la numérotation des lignes du fichier source ?

Dans la règle `<COMMENTAIRE>\n`, il suffit d'ajouter un `ECHO` (ou `putchar('\n')`). Le commentaire disparaît, mais ses sauts de ligne restent. Chaque ligne de code garde alors **le même numéro** que dans le fichier source, ce qui est utile pour les messages d'erreur d'un compilateur.

### Q26. Quel est le lien avec le préprocesseur C (`cpp`) ? Quelle limite a notre analyseur ?

Supprimer les commentaires est une étape du **prétraitement**, qui a lieu avant l'analyse lexicale proprement dite. Les commentaires n'ont aucun sens pour la grammaire du langage. Le compilateur s'en débarrasse donc au plus tôt, en général en les remplaçant par **un espace** pour que deux mots ne se collent pas (`a/**/b` donne `a b` et non `ab`).

C'est d'ailleurs une limite de notre analyseur : il produirait `ab`. Pour la corriger, on peut écrire `putchar(' ');` dans l'action de `"*/"`.
