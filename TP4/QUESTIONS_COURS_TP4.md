# TP4 – Implémentation d'automates : Questions de cours et réponses

Fichiers concernés : [tp4.c](tp4.c), [test.txt](test.txt), [resultat.txt](resultat.txt), [README.md](README.md)
Énoncé : `TPS/tp4.pdf`

Les questions suivent les 9 étapes du TP :

| Partie | Thème |
|---|---|
| 1 | L'analyseur lexical : lexème, unité lexicale |
| 2 | Les 6 unités et leurs définitions régulières |
| 3 | L'automate fini déterministe (AFD) |
| 4 | Construire les AFD du TP4 |
| 5 | Le plus long préfixe et le retour en arrière |
| 6 | Le tampon et les deux pointeurs |
| 7 | Programmer un AFD avec `switch/case` |
| 8 | `Echec()` et l'ordre des automates |
| 9 | Les erreurs lexicales et le déroulement sur `test.txt` |
| Bonus | Lien avec le cours et les autres TP |

---

## Partie 1 : l'analyseur lexical

### Q1. Quel est le rôle d'un analyseur lexical ?

C'est la **première phase** d'un compilateur. Il lit le programme source **caractère par caractère** et regroupe ces caractères en **lexèmes**. Pour chaque lexème, il fournit l'**unité lexicale** (le token) correspondante.

Il **saute aussi les blancs** (espaces, tabulations), qui ne sont pas des unités lexicales.

### Q2. Quelle est la différence entre lexème, unité lexicale et modèle ?

| Terme | Définition | Exemple |
|---|---|---|
| **Lexème** | le morceau de texte exact trouvé dans le source | `x1`, `<=`, `12.5` |
| **Unité lexicale (token)** | la **catégorie** du lexème | `ID`, `OPREL`, `NB` |
| **Modèle** | la **règle** qui décrit tous les lexèmes d'une unité | `lettre (lettre \| chiffre)*` |

Plusieurs lexèmes peuvent avoir la même unité : `x1` et `y` sont tous les deux des `ID`.

### Q3. Découper `si a >= 10` en lexèmes et unités lexicales.

| Lexème | Unité |
|---|---|
| `si` | SI |
| `a` | ID |
| `>=` | OPREL |
| `10` | NB |

### Q4. Que demande le TP4 ? Quel est le lien avec le TP3 ?

Le TP4 demande de construire **à la main** un analyseur lexical en C pour les unités `si`, `alors`, `sinon`, `oprel`, `id` et `nb`, en deux étapes :
1. dessiner les **diagrammes de transitions déterministes (AFD)** ;
2. les **implémenter** avec un `CASE` (`switch`) et une fonction **`Echec`**.

Dans le TP3, **Flex** générait automatiquement ce code à partir des expressions régulières. Le TP4 montre donc **ce que Flex fait à l'intérieur**.

---

## Partie 2 : les unités et les définitions régulières

### Q5. Que signifient les symboles `|`, `*`, `+`, `?` et `( )` dans une définition régulière ?

| Symbole | Sens | Exemple |
|---|---|---|
| `ab` | `a` puis `b` (concaténation) | `si` |
| `\|` | ou | `< \| >` |
| `*` | 0, 1 ou plusieurs fois | `chiffre*` (accepte le vide) |
| `+` | 1 fois ou plus | `chiffre+` (au moins un chiffre) |
| `?` | facultatif (0 ou 1 fois) | `(+ \| -)?` |
| `( )` | regroupement | `(lettre \| chiffre)*` |

### Q6. Donner les 6 définitions régulières du TP4.

```
si    → si
alors → alors
sinon → sinon
oprel → <  |  <=  |  =  |  <>  |  >  |  >=
id    → lettre (lettre | chiffre)*
nb    → chiffre+ (. chiffre+)? (E (+ | -)? chiffre+)?
```

### Q7. Que signifie « on supposera les mots-clés réservés » ?

Les mots `si`, `alors` et `sinon` **ne peuvent pas** servir d'identificateurs. Le texte `si` donne **toujours** le token SI, jamais ID, même s'il respecte aussi la règle de `id`.

### Q8. Découper la règle de `nb` en ses 3 parties.

| Partie | Morceau | Obligatoire ? | Exemple |
|---|---|---|---|
| entière | `chiffre+` | **oui** | `12` |
| décimale | `(. chiffre+)?` | non | `.5` |
| exposant | `(E (+\|-)? chiffre+)?` | non | `E-3` |

`12.5E-3` signifie 12,5 × 10⁻³.

### Q9. Ces lexèmes sont-ils valides ? Lesquels ?

| Texte | Résultat | Raison |
|---|---|---|
| `alors2` | ID | `alors` est suivi d'un chiffre, donc ce n'est pas le mot-clé |
| `>=` | OPREL | |
| `45E+10` | NB | entier, puis exposant avec signe |
| `9.` | invalide en entier | après le `.`, il faut au moins un chiffre. L'analyseur donne NB `9` puis une erreur sur `.` |
| `b7` | ID | |
| `.5` | invalide | la partie entière est obligatoire |
| `3E` | invalide en entier | après `E`, il faut au moins un chiffre. On obtient NB `3`, puis ID `E` |
| `1e5` | invalide en entier | l'énoncé impose `E` **majuscule**. On obtient NB `1`, puis ID `e5` |
| `007.50` | NB | les zéros de tête sont des chiffres comme les autres |

---

## Partie 3 : l'automate fini déterministe (AFD)

### Q10. Qu'est-ce qu'un automate fini ? Donner ses éléments.

C'est une « machine » qui lit un mot **caractère par caractère** en suivant des flèches. Elle se compose de :
- un ensemble d'**états** ;
- un **état initial** (le point de départ) ;
- des **états finaux** (d'acceptation) ;
- des **transitions** : `état --caractère--> état`.

Le mot est **accepté** si, après avoir lu tous ses caractères, on se trouve sur un état final. Il est **refusé** si l'automate est bloqué (aucune transition possible) ou s'il termine sur un état non final.

### Q11. Que signifie « déterministe » ? Pourquoi est-ce important ici ?

Depuis chaque état, il y a **au plus une transition par caractère**. Le chemin suivi est donc **unique** : il n'y a jamais de choix à faire, ni de retour en arrière pour explorer une autre possibilité.

C'est ce qui rend l'AFD **facile à programmer** (un `switch` suffit) et **rapide** : le temps d'analyse est proportionnel à la longueur du texte.

### Q12. Sur l'automate de `id` (19 → 20, avec une boucle sur 20), les mots `a` et `9` sont-ils acceptés ?

- `a` : de 19, on lit une lettre et on passe à 20. Le mot est fini et 20 est final, donc **accepté**.
- `9` : depuis 19, il n'y a pas de transition « chiffre ». L'automate est **bloqué**, donc le mot est **refusé**. C'est l'automate `nb` qui reconnaîtra `9`.

### Q13. Pourquoi l'état initial 19 n'est-il pas final ?

S'il l'était, l'automate accepterait le **mot vide ε**, sans lire aucun caractère. Or un identificateur doit contenir **au moins une lettre**.

Règle générale : l'état initial n'est final que si le mot vide fait partie du langage. Dans le TP4, aucun état initial (0, 3, 9, 15, 19, 21) n'est donc final.

### Q14. Quelles sont les deux façons de représenter un AFD ?

1. Le **diagramme de transitions** : des cercles et des flèches.
2. La **table de transitions** : une ligne par état, une colonne par type de caractère, et une colonne « final ? ».

Les deux contiennent exactement la même information. La table est plus proche du code.

---

## Partie 4 : construire les AFD du TP4

### Q15. Comment chaque symbole d'une définition régulière se traduit-il dans un automate ?

| Règle | Automate |
|---|---|
| `ab` | une chaîne : `(0)─a→(1)─b→(2)` |
| `a \| b` | deux flèches qui partent du même état |
| `a*` | une boucle sur l'état |
| `a+` | une flèche, puis une boucle sur l'état d'arrivée |
| `x?` | un chemin facultatif : l'état d'avant est final, ou une flèche permet de « sauter » |

### Q16. Dessiner les automates des mots-clés.

```
si    : →(0) ─s→ (1) ─i→ [2]
alors : →(3) ─a→ (4) ─l→ (5) ─o→ (6) ─r→ (7) ─s→ [8]
sinon : →(9) ─s→ (10) ─i→ (11) ─n→ (12) ─o→ (13) ─n→ [14]
```

Les numéros se suivent d'un automate à l'autre, pour qu'on ne confonde pas les états de deux automates.

### Q17. Quel problème pose l'automate de `si` sur le texte `sinon` ? Comment le résout-on ?

Après avoir lu `s` puis `i`, l'automate est sur l'état final 2. Il dirait donc « SI », et le reste `non` deviendrait un ID. C'est faux.

**Solution :** un mot-clé n'est accepté que si le **caractère suivant n'est ni une lettre ni un chiffre** (un délimiteur). C'est le rôle de `fin_mot()`. Pour la même raison, `sinonx` et `si1` deviennent des ID.

### Q18. Dessiner l'automate de `oprel`. Quels états sont finaux ?

```
 →(15) ─<→ [16] ─(= ou >)→ [18]        <   <=   <>
 →(15) ─>→ [17] ─=→ [18]               >   >=
 →(15) ─=→ [18]                        =
```

Les états finaux sont **16, 17 et 18**. Les états 16 et 17 sont finaux parce que `<` et `>` sont valides **seuls**, mais ils ont encore des transitions sortantes, car l'opérateur peut continuer (`<=`).

### Q19. Donner la table de transitions de `nb`.

| État | chiffre | `.` | `E` | `+`/`-` | Final ? | Signification |
|---|---|---|---|---|---|---|
| 21 | 22 | | | | non | départ |
| 22 | 22 | 23 | 25 | | **oui** | partie entière |
| 23 | 24 | | | | non | point lu, chiffre attendu |
| 24 | 24 | | 25 | | **oui** | partie décimale |
| 25 | 27 | | | 26 | non | `E` lu |
| 26 | 27 | | | | non | signe lu |
| 27 | 27 | | | | **oui** | exposant |

### Q20. Pourquoi l'état 23 n'est-il pas final, alors que 22 et 24 le sont ?

- En **22**, on a lu des chiffres (`12`) : c'est un nombre complet.
- En **23**, on a lu `12.` : la règle `(. chiffre+)` exige **au moins un chiffre** après le point. Le nombre est donc incomplet.
- En **24**, on a lu `12.5` : c'est complet.

Pour la même raison, **25** (`3E`) et **26** (`3E+`) ne sont pas finaux.

### Q21. Faire le parcours de `45E+10` dans l'automate `nb`.

`21 ─4→ 22 ─5→ 22 ─E→ 25 ─+→ 26 ─1→ 27 ─0→ 27`, fin du mot.

On termine sur l'état 27, qui est final : le mot est **accepté**.

### Q22. Pourquoi peut-on aller de 25 à 27 directement ?

Parce que le signe est **facultatif** : `(+ | -)?`. La transition `25 ─chiffre→ 27` correspond au cas sans signe (`3E8`). La transition `25 ─+/-→ 26 ─chiffre→ 27` correspond au cas avec signe (`3E+8`).

---

## Partie 5 : le plus long préfixe et le retour en arrière

### Q23. Pourquoi l'analyseur doit-il décider lui-même où s'arrête un lexème ?

Parce que les lexèmes ne sont pas toujours séparés par des espaces : dans `a>=10`, tout est collé. L'automate doit trouver lui-même la frontière de chaque lexème.

### Q24. Énoncer la règle du plus long préfixe.

> On avance tant qu'une transition existe. À chaque passage sur un état final, on **mémorise** la position (le « dernier succès »). Quand on est bloqué, on **revient** au dernier succès : le lexème est le **plus long préfixe** reconnu.

C'est la même règle que le « longest match » de Flex.

### Q25. Pourquoi prendre le plus long préfixe plutôt que le premier trouvé ?

Si on s'arrêtait au premier état final :
- `>=` donnerait `>` puis une erreur sur `=` ;
- `x12` donnerait ID `x` puis NB `12` ;
- `12.5` donnerait `1`, `2`…

Le plus long préfixe donne le découpage **naturel**.

### Q26. Que renvoie `oprel` sur `<>5` ? Décrire les états.

| Lu | État | Dernier succès |
|---|---|---|
| `<` | 16 (final) | `<` |
| `>` | 18 (final) | `<>` |
| `5` | bloqué | |

Le résultat est **OPREL `<>`**. Le `5` est **rendu**, puis relu au tour suivant, ce qui donne NB `5`.

### Q27. Que renvoie `nb` sur `7.5E` ? Que devient le reste ?

On passe par `7` (22 ✓), `.` (23), `5` (24 ✓), `E` (25), puis on est bloqué. Le dernier succès est `7.5`, donc **NB `7.5`**.

Le `E` est relu seul au tour suivant. Il est reconnu par l'automate `id`, ce qui donne **ID `E`**.

### Q28. Pourquoi `007.50` donne-t-il un seul NB ?

Pour deux raisons :
- les **boucles** des états 22 et 24 permettent de lire plusieurs chiffres à la suite ;
- la règle du **plus long préfixe** continue jusqu'au bout au lieu de s'arrêter au premier `0`.

### Q29. Que donne `2.` en fin de ligne ?

L'automate passe par `2` (22 ✓), puis `.` (23, non final), puis la fin de ligne : bloqué. Il revient à `2`, ce qui donne **NB `2`**. Le `.` est relu seul : aucun automate ne le reconnaît, d'où une **erreur lexicale**.

---

## Partie 6 : le tampon et les deux pointeurs

### Q30. Qu'est-ce que le tampon d'entrée ? Pourquoi en a-t-on besoin ?

C'est un **tableau de caractères en mémoire** qui contient le texte à analyser. Ici, c'est une ligne lue avec `fgets` et pointée par `buf`.

On en a besoin pour pouvoir **revenir en arrière**. Avec `getchar()`, un caractère lu est consommé, et le relire est difficile (`ungetc` ne garantit qu'un seul caractère). Avec un tableau, reculer revient simplement à **changer un indice**.

### Q31. Quel est le rôle des deux pointeurs `debut` et `avance` ?

| Pointeur | Rôle |
|---|---|
| `debut` | indice du **début** du lexème en cours. Il est fixe pendant la recherche |
| `avance` | indice du **prochain caractère à lire**. Il avance à chaque `lire()` |

Le lexème en cours est `buf[debut .. avance[`.

### Q32. Comment fait-on concrètement un retour en arrière ?

On **réécrit la valeur de `avance`** :
- après un succès de longueur `len` : `avance = debut + len;` ;
- après un échec : `avance = debut;` (dans `Echec()`).

### Q33. Sur le tampon `x<=5`, donner `debut` et `avance` à chaque étape.

| Moment | `debut` | `avance` |
|---|---|---|
| départ | 0 | 0 |
| `id` lit `x`, puis `<` (bloqué) et renvoie 1 | 0 | 2 |
| après le retour en arrière (`avance = debut + 1`) | 0 | **1** |
| nouveau lexème | **1** | 1 |
| `oprel` lit `<`, `=`, puis `5` (bloqué) et renvoie **2** | 1 | 4 |
| après le retour en arrière | 1 | **3** |
| nouveau lexème : `nb` reconnaît `5` | 3 | 4 |

### Q34. Pourquoi y a-t-il un `- 1` dans `dernier = avance - 1 - debut` ?

Dans la boucle, on lit **d'abord** le caractère (`c = lire()`, qui fait `avance++`), **puis** on traite l'état. Quand on note le succès d'un état final, `avance` est déjà **après** le caractère `c`, alors que `c` ne fait pas (encore) partie du lexème reconnu. La longueur du succès est donc `(avance - 1) - debut`.

---

## Partie 7 : programmer un AFD avec `switch/case`

### Q35. Donner le modèle général du code d'un AFD.

```c
int etat = ETAT_INITIAL, c, dernier = -1;
for (;;) {
    c = lire();
    switch (etat) {
    case A:
        dernier = avance - 1 - debut;     /* seulement si A est final */
        if (condition sur c) etat = B;    /* une transition */
        else return dernier;              /* blocage */
        break;
    ...
    }
}
```

### Q36. À quoi correspondent un `case`, un `if` dans un `case`, et `return dernier` ?

| Code | Automate |
|---|---|
| `case N:` | l'état N (une ligne de la table) |
| `if (...) etat = M;` | une transition N → M |
| `dernier = ...` | N est un état **final** |
| `else return dernier;` | aucune transition : **blocage**, retour au dernier succès |
| pas de `etat = ...` | on reste dans le même état (boucle) |

### Q37. Comment reconnaît-on un état final dans le code de `afd_nb` ?

Son `case` commence par `dernier = avance - 1 - debut;`. C'est le cas des états 22, 24 et 27. Les états non finaux (21, 23, 25, 26) ne modifient pas `dernier`.

### Q38. Que renvoie `afd_id()` si le premier caractère est `5` ? Pourquoi ?

Elle renvoie **-1**. En `case 19`, `isalpha('5')` est faux, donc on exécute `return dernier`, et `dernier` vaut encore sa valeur initiale -1. Cela signifie « échec : rien n'a été reconnu ».

### Q39. Écrire le `case 23` de `nb` (23 : chiffre → 24, non final).

```c
case 23:
    if (isdigit(c)) etat = 24;
    else return dernier;     /* pas de "dernier = ..." : 23 n'est pas final */
    break;
```

### Q40. Pourquoi l'état 18 de `oprel` fait-il directement `return` ?

18 est final et **n'a aucune transition sortante**. Une fois qu'on y est, aucun caractère ne peut prolonger le lexème : on renvoie le succès tout de suite.

### Q41. Pourquoi les mots-clés utilisent-ils une fonction générique `afd_motcle(mot)` ?

Leurs automates sont de **simples chaînes**. Une boucle `for` sur les lettres du mot fait exactement le même travail : l'indice `i` joue le rôle de l'état. Cela évite d'écrire trois `switch` presque identiques. On l'appelle avec `"si"`, `"alors"` ou `"sinon"`.

### Q42. Que signifie la valeur renvoyée par un automate ?

| Valeur | Signification |
|---|---|
| `-1` | échec : rien n'a été reconnu à partir de `debut` |
| `len > 0` | succès : le lexème est `buf[debut .. debut + len[` |

---

## Partie 8 : la fonction `Echec()` et l'ordre des automates

### Q43. Pourquoi a-t-on besoin d'une fonction `Echec()` ?

Chacun des 6 automates ne reconnaît **qu'une** unité, et au début d'un lexème on ne sait pas laquelle va suivre. On les essaie donc **l'un après l'autre**. `Echec()` assure le **branchement d'un automate au suivant** quand le courant échoue.

### Q44. Que fait `Echec()` ?

```c
static int Echec(void) {
    avance = debut;      /* 1. retour au début du lexème */
    return ++automate;   /* 2. passage à l'automate suivant */
}
```

Dans le cours (Aho), `Echec()` renvoie l'**état initial** de l'automate suivant : 0 → 3 → 9 → 15 → 19 → 21.

### Q45. Pourquoi faut-il remettre `avance = debut` ?

L'automate qui a échoué a peut-être déjà lu plusieurs caractères. Par exemple, sur `sin`, l'automate `sinon` a lu `s`, `i` et `n`. Sans retour au début, l'automate suivant (`id`) commencerait **au milieu du mot**, et ne verrait jamais `sin` en entier.

Dans `tp4.c`, la boucle de `prochain()` refait aussi `avance = debut` avant chaque automate. C'est une **double sécurité**.

### Q46. Dans quel ordre les automates sont-ils essayés ? Pourquoi ?

L'ordre est **si → alors → sinon → oprel → nb → id**.
- **Les mots-clés doivent passer avant `id`** : `si` est reconnu à la fois par `afd_motcle("si")` et par `afd_id`. Comme les mots-clés sont réservés, ils doivent gagner. C'est la même règle qu'en Flex, où les mots-clés s'écrivent avant la règle des identificateurs.
- L'ordre entre `oprel`, `nb` et `id` est sans importance : ils ne commencent jamais par le même caractère (`<`, `>` ou `=` pour oprel, un chiffre pour nb, une lettre pour id).

### Q47. Lister les automates essayés pour `alors1`.

| Automate | Résultat |
|---|---|
| si | ❌ (`a` au lieu de `s`) |
| alors | ❌ (`alors` est suivi de `1` : ce n'est pas un délimiteur) |
| sinon | ❌ |
| oprel | ❌ |
| nb | ❌ |
| id | ✅ **ID `alors1`** |

### Q48. Que donnerait `si x > 3 alors y` si `afd_id` était placé en premier ?

```
ID <si>  ID <x>  OPREL <>>  NB <3>  ID <alors>  ID <y>
```

Les mots-clés ne seraient **jamais** reconnus.

### Q49. Pourquoi ne pas fusionner `si` et `sinon` en un seul automate ?

On pourrait : `(0)─s→(1)─i→[2]─n→(12)─o→(13)─n→[14]`. Ce serait plus efficace, car `s` et `i` ne seraient lus qu'une fois. Mais l'énoncé demande **un diagramme par terminal**, enchaînés avec `Echec()`. C'est plus simple à construire et à vérifier.

---

## Partie 9 : erreurs lexicales et déroulement complet

### Q50. Qu'est-ce qu'une erreur lexicale ? Donner des exemples.

C'est un caractère qui ne commence **aucune** unité lexicale : les 6 automates échouent. Exemples dans ce langage :
- `@` et `#` ;
- `+` seul (il n'existe que **dans** un exposant) ;
- `.` seul (il n'existe que **dans** un nombre).

### Q51. Comment l'analyseur réagit-il à une erreur ?

Il utilise la **récupération en mode panique** :
- il **signale** l'erreur, avec le caractère et sa colonne ;
- il **saute un seul caractère** (`avance = debut + 1`) ;
- il **continue** l'analyse.

### Q52. Pourquoi ne sauter qu'un seul caractère ?

Pour **perdre le moins de texte possible** et pouvoir signaler **toutes** les erreurs en une seule exécution. Dans `@ 007.50`, le nombre `007.50` est encore reconnu après l'erreur sur `@`.

### Q53. Pourquoi affiche-t-on `col + 1` ?

L'indice dans le tampon commence à **0**, alors que les colonnes affichées à l'utilisateur commencent à **1**.

### Q54. Dans `4E+`, pourquoi `E` est-il un ID et `+` une erreur ?

L'automate `nb` lit `4` (22 ✓), puis `E` (25), puis `+` (26), puis il est bloqué. Il revient au dernier succès, d'où **NB `4`**.

On analyse ensuite ce qui reste :
- `E` est une **lettre**, donc il est reconnu par `id` : **ID `E`** ;
- `+` ne commence aucune unité (ce n'est ni un oprel, ni un chiffre, ni une lettre) : **ERREUR `+`**.

### Q55. Que produit l'analyseur sur `si x=.5 alors` ?

```
SI     <si>
ID     <x>
OPREL  <=>
ERREUR lexicale : caractere '.' (col 6)
NB     <5>
ALORS  <alors>
```

Le `.5` n'est pas un nombre, car la partie entière est obligatoire.

### Q56. Expliquer le résultat de la ligne 3 de `test.txt` : `sinonx alors1 sin 3E 4E+ @ 007.50 #`

| Texte | Résultat | Raison |
|---|---|---|
| `sinonx` | ID | `sinon` est suivi de `x`, donc ce n'est pas le mot-clé |
| `alors1` | ID | même chose avec un chiffre |
| `sin` | ID | `sinon` attend un `o`, échec, puis `id` réussit |
| `3E` | NB `3`, ID `E` | plus long préfixe et retour en arrière |
| `4E+` | NB `4`, ID `E`, ERREUR `+` (col 24) | voir Q54 |
| `@` | ERREUR (col 26) | aucun automate |
| `007.50` | NB | l'analyse a repris après l'erreur |
| `#` | ERREUR (col 35) | aucun automate |

### Q57. Que donne `a<>=b` ? Et `=<` ?

- `a<>=b` donne ID `a`, OPREL `<>`, OPREL `=`, ID `b`. L'état 18 n'a pas de transition sortante : `<>` est terminé, et `=` est un nouveau lexème.
- `=<` donne OPREL `=`, puis OPREL `<`. `=<` n'est pas un opérateur du langage.

### Q58. Décrire l'architecture complète du programme.

```
main()                     lit le fichier ligne par ligne (tampon)
 └─ prochain()             saute les blancs, debut = avance
     └─ switch (automate)  essaie les automates dans l'ordre
         ├─ afd_motcle / afd_oprel / afd_nb / afd_id
         │    └─ switch (etat), plus long préfixe
         ├─ Echec()        retour au début + automate suivant
         └─ erreur         tout a échoué : on saute 1 caractère
```

### Q59. Comment compiler et exécuter le TP4 ?

```powershell
gcc -Wall -o tp4 tp4.c
.\tp4.exe test.txt
```

Ou bien `make test`, avec `C:\msys64\usr\bin` dans le PATH.

---

## Bonus : lien avec le cours et les autres TP

### Q60. Comparer l'analyseur écrit à la main (TP4) avec Flex (TP3).

| | TP4 (à la main) | TP3 (Flex) |
|---|---|---|
| on écrit | les AFD, le `switch`, `Echec()`, le tampon | seulement les expressions régulières et les actions |
| construction de l'AFD | à la main | automatique (expression → AFN → AFD minimal) |
| plus long préfixe | codé avec `dernier` | intégré |
| priorité des mots-clés | ordre des automates | ordre des règles |
| ajouter un mot-clé | nouvel automate + nouveau `case` | une seule ligne |

Les **principes sont les mêmes** : plus long préfixe, priorité à la première règle, retour en arrière. Flex les automatise.

### Q61. Comment passe-t-on d'une expression régulière à un AFD en théorie ?

1. On construit un **AFN** à partir de l'expression (construction de Thompson).
2. On le transforme en **AFD** par la construction des sous-ensembles.
3. On **minimise** l'AFD.

Dans le TP4, on a construit directement les AFD « à l'œil », car les expressions sont simples.

### Q62. À quoi sert la grammaire donnée dans l'énoncé (`instr → si expr alors instr ...`) ?

Elle n'est **pas** utilisée par l'analyseur lexical. Elle décrit la **syntaxe**, c'est-à-dire la façon dont les tokens s'enchaînent. C'est l'**analyseur syntaxique** (TP5) qui l'utilisera : il appellera `prochain()` pour obtenir les tokens un par un et vérifiera qu'ils respectent la grammaire.

L'analyseur lexical s'occupe des **mots**, l'analyseur syntaxique de la **phrase**.
