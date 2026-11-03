# TP3 — Comprendre l'exercice 1.1 pas à pas

Ce document explique le rôle de l'entrée, le traitement effectué par Flex, la sortie obtenue et les commandes pour lancer le programme. Il concerne **l'exercice 1.1** de l'énoncé. Les exercices 1.2 et 1.3 demandent d'autres analyseurs.

## 1. Objectif

L'objectif est de construire un **analyseur lexical**. Celui-ci lit le texte d'un programme source et le découpe en éléments reconnaissables, appelés *unités lexicales* ou *tokens* : mots-clés, identificateurs, opérateurs, nombres et séparateurs.

Il ne lance pas le programme source. Par exemple, lorsqu'il lit une boucle `for`, il reconnaît le mot `for` et les symboles qui suivent, mais il n'exécute pas la boucle. Il ne vérifie pas non plus que toutes les instructions respectent la grammaire du langage.

L'exercice progresse en trois étapes :

1. **1.1 a — Identificateurs :** reconnaître les mots commençant par une lettre et pouvant continuer par des lettres ou des chiffres. Dans la spécification de départ de l'énoncé, ils sont affichés sous la forme `<mot>`.
2. **1.1 b1 — Catégories supplémentaires :** distinguer les mots-clés, les identificateurs, les opérateurs et les caractères réservés.
3. **1.1 b2 — Nombres :** reconnaître aussi les valeurs entières et flottantes.

Le fichier `exo1.l` du projet correspond à la version enrichie de l'exercice 1.1 : il réalise les étapes b1 et b2 en plus de la reconnaissance des identificateurs.

## 2. Entrée : que lit l'analyseur ?

L'entrée est un **fichier texte** contenant un exemple de programme source. L'énoncé l'appelle `0.dat` ; dans ce projet, le fichier de test est `exo1.dat`. Son extension `.dat` ne signifie pas qu'il faut l'exécuter : c'est seulement le texte que l'analyseur doit lire.

Voici le début de `exo1.dat` :

```c
for( i=0; i < 10; i++ ) {
    truc[i] = 0;
    machin = 12.3;
    bidule = .0;
    valeur = 45.67e-2;
```

Cet extrait s'arrête avant la fin du fichier réel. Tu peux ouvrir `exo1.dat` et remplacer son contenu pour essayer d'autres exemples.

## 3. Avant le traitement : le rôle de chaque fichier

| Fichier | Rôle |
| --- | --- |
| `exo1.l` | Contient les expressions régulières et les actions à effectuer lorsqu'un élément est reconnu. C'est le fichier que l'on écrit pour l'analyseur. |
| `exo1.dat` | Contient le programme source à analyser. |
| `lex.yy.c` | Code C généré automatiquement par Flex à partir de `exo1.l`. |
| `exo1.exe` | Programme obtenu après compilation de `lex.yy.c` sous Windows. Il lit `exo1.dat` et affiche le résultat. |
| `Makefile` | Regroupe les commandes de génération, compilation et test. Il est pratique, mais l'énoncé ne demande pas de le créer. |

Le parcours complet est donc :

```text
exo1.l --Flex--> lex.yy.c --GCC--> exo1.exe
                                      |
                                      +-- lit exo1.dat --> affiche les éléments reconnus
```

Dans `exo1.l`, la fonction `main` ouvre le fichier passé en argument, l'affecte à `yyin`, puis appelle `yylex()`. C'est `yylex()` qui parcourt l'entrée et applique les règles.

## 4. Traitement : que se passe-t-il pendant la lecture ?

L'analyseur parcourt le texte de gauche à droite. À chaque position, Flex cherche quelle expression régulière correspond aux caractères qui suivent. Lorsqu'une règle est choisie, son action C s'exécute. Ici, les actions utilisent surtout `printf` pour afficher la catégorie et le texte reconnu. Ce texte reconnu est disponible dans la variable `yytext`.

Suivons `for( i=0;` pas à pas :

| Étape | Texte rencontré | Règle utilisée | Résultat affiché |
| --- | --- | --- | --- |
| 1 | `for` | Mot-clé `for` | `[MOT_CLE: for]` |
| 2 | `(` | Parenthèse ouvrante | `[PAR_OUVR: (]` |
| 3 | espace | Espace | Rien : il est ignoré |
| 4 | `i` | Identificateur | `[IDF: i]` |
| 5 | `=` | Affectation | `[AFFECT: =]` |
| 6 | `0` | Nombre entier | `[NB_ENTIER: 0]` |
| 7 | `;` | Point-virgule | `[PVIRG: ;]` |

Le traitement recommence ensuite à la position suivante, jusqu'à la fin du fichier.

### Comment Flex choisit-il entre plusieurs règles ?

1. **Il choisit d'abord le texte reconnu le plus long.** Ainsi, `>=` devient un seul opérateur `[OP_GE: >=]`, et `12.3` devient un seul réel `[NB_REEL: 12.3]`.
2. **En cas d'égalité de longueur, la première règle écrite dans `exo1.l` gagne.** `for` correspond à la fois à la règle du mot-clé et à celle des identificateurs ; la règle du mot-clé, placée avant, produit `[MOT_CLE: for]`.

Cela explique aussi pourquoi `format` reste un identificateur : l'identificateur reconnaît les six caractères, tandis que la règle de `for` n'en reconnaîtrait que trois. Flex retient la correspondance la plus longue.

La règle des espaces les ignore. La règle des retours à la ligne les réaffiche. Les règles des accolades ajoutent elles-mêmes un retour à la ligne ; on peut donc voir des lignes vides dans la sortie. Enfin, la règle `.` affiche `[INCONNU: ...]` pour un caractère qui n'est reconnu par aucune des autres règles.

## 5. Sortie : que doit-on obtenir ?

La sortie est une **suite d'éléments étiquetés**. Pour `for( i=0;`, on obtient :

```text
[MOT_CLE: for] [PAR_OUVR: (] [IDF: i] [AFFECT: =] [NB_ENTIER: 0] [PVIRG: ;]
```

Dans le fichier de test actuel, on trouve aussi les exemples suivants :

```text
[IDF: machin] [AFFECT: =] [NB_REEL: 12.3] [PVIRG: ;]
[IDF: bidule] [AFFECT: =] [NB_REEL: .0] [PVIRG: ;]
[IDF: valeur] [AFFECT: =] [NB_REEL: 45.67e-2] [PVIRG: ;]
```

La sortie s'affiche normalement **dans le terminal**. L'énoncé ne demande pas de créer un fichier de sortie. Si tu souhaites conserver le résultat, tu peux le rediriger vers un fichier texte ; voir les commandes ci-dessous.

### Ce qu'il faut expliquer dans le compte rendu

- Dans la **première spécification** de l'énoncé (1.1 a), `for` est affiché comme un mot ordinaire, par exemple `<for>`. Il n'existe pas encore de règle qui le classe comme mot-clé. Les caractères sans règle particulière sont recopiés par le comportement par défaut de Flex.
- Dans la **version enrichie** du projet, `for` est affiché comme mot-clé, `i` comme identificateur, `10` comme entier et `12.3` comme réel.
- Les opérateurs composés comme `++` et `>=` sont reconnus comme un seul élément grâce à la règle de la correspondance la plus longue.
- Les espaces ne produisent pas de catégorie affichée ; les retours à la ligne influencent seulement la présentation de la sortie.
- L'analyseur décrit les morceaux du texte. Il ne calcule pas les expressions et ne dit pas si le programme entier est syntaxiquement correct.

## 6. Commandes pour exécuter le programme

Place-toi d'abord dans le dossier `TP3`. Les commandes suivantes sont destinées à **PowerShell sous Windows**, comme le terminal qui affiche `PS C:\...\TP3>`.

### Option A — Utiliser le Makefile

Si `make`, `flex` ou `gcc` ne sont pas reconnus, ajoute les dossiers MSYS2 au `PATH` de la fenêtre PowerShell actuelle :

```powershell
$env:Path = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:Path"
make test
```

`make test` génère le code C si nécessaire, compile le programme si nécessaire, puis lance `exo1` sur `exo1.dat`. La modification du `PATH` ne vaut que pour cette fenêtre : il faut la refaire dans une nouvelle fenêtre PowerShell. La commande est `make test`, et non `make file`.

### Option B — Sans Makefile

Exécute les commandes **une par une** dans PowerShell :

```powershell
& "C:\msys64\usr\bin\flex.exe" exo1.l
& "C:\msys64\ucrt64\bin\gcc.exe" lex.yy.c -o exo1.exe
.\exo1.exe exo1.dat
```

Si tu modifies seulement `exo1.dat`, il suffit ensuite de relancer `.\exo1.exe exo1.dat`. Si tu modifies `exo1.l`, régénère et recompile avant de relancer l'exécutable.

L'ancienne version de PowerShell utilisée ici ne reconnaît pas `&&` comme séparateur : utilise les lignes séparées ci-dessus.

### Option C — Conserver la sortie dans un fichier

Après la compilation, cette commande écrit les éléments reconnus dans `resultat.txt` au lieu de les afficher dans le terminal :

```powershell
.\exo1.exe exo1.dat > resultat.txt
```

Le fichier de résultat est facultatif. Il ne faut pas essayer d'exécuter `exo1.dat` directement : c'est l'entrée de l'analyseur, pas un programme exécutable.

## 7. À retenir

```text
Entrée : texte du programme dans exo1.dat
Traitement : règles lexicales de exo1.l appliquées par l'analyseur généré par Flex
Sortie : mots, nombres et symboles reconnus, avec leur catégorie
```

L'exercice 1.1 montre comment passer d'un texte brut à des unités lexicales. Cette étape servira ensuite à l'analyse syntaxique d'un compilateur.
