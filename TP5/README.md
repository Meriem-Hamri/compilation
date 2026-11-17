# TP5 - Grammaire des expressions régulières, LL(1) et mode panique

Binôme : HAMRI Meriem, BAKKARI Zakariae
Compilation / test : `gcc -Wall -o tp5 tp5.c && ./tp5 test.txt`

## 1) G est ambiguë
`R -> R R | R + R | R * | ( R ) | a | b`

Mot `a+bb` : deux arbres de dérivation.
- Arbre 1 : `R -> R + R` avec `R1 = a` et `R2 = bb` (`R2 -> R R`) : a + (bb).
- Arbre 2 : `R -> R R` avec `R1 = a+b` (`R -> R + R`) et `R2 = b` : (a+b)b.

L'ambiguïté est de **priorité** : `+` et la concaténation ne sont pas hiérarchisés.
Le sens change : le 1er arbre désigne {a, bb}, le 2e {ab, bb}. (Il y a aussi une
ambiguïté d'associativité : `abc` = (ab)c ou a(bc).)

## 2) Grammaire G' non ambiguë (`*` > concaténation > `+`)
```
R -> R + T | T
T -> T F   | F
F -> F *   | P
P -> ( R ) | a | b
```
Pour `a+bb`, le seul arbre est `R -> R + T` avec `R -> T -> F -> P -> a`
et `T -> T F` (`T -> F -> P -> b`, `F -> P -> b`) : la concaténation `bb` est
forcément regroupée sous `T` avant le `+`.

## 3) G' rendue LL(1)
Élimination de la récursivité à gauche (`A -> A α | β` => `A -> β A'`, `A' -> α A' | ε`).
Pas de factorisation nécessaire.
```
1) R  -> T R'     2) R' -> + T R'   3) R' -> ε
4) T  -> F T'     5) T' -> F T'     6) T' -> ε
7) F  -> P F'     8) F' -> * F'     9) F' -> ε
10) P -> ( R )   11) P  -> a       12) P  -> b          (S -> R $)
```
| | Premier | Suivant |
|---|---|---|
| R | ( a b | ) $ |
| R' | + ε | ) $ |
| T | ( a b | + ) $ |
| T' | ( a b ε | + ) $ |
| F | ( a b | ( a b + ) $ |
| F' | * ε | ( a b + ) $ |
| P | ( a b | * ( a b + ) $ |

Table sans conflit, donc G' est LL(1) (voir `resultat.txt`, la table est affichée en tête).

## 4) Simulation de l'analyseur LL(1) (`tp5.c`)
Algorithme du cours avec une pile. La table est construite par le programme à partir
de Premier/Suivant. **Mode panique** :
- `M[A,a]` vide : si `a` ∈ Suivant(A) on dépile A, sinon on saute `a` ;
- terminal attendu ≠ terminal lu : on dépile le terminal (message) ;
- entrée restante après l'axiome : on saute les symboles en trop ;
- caractère inconnu : erreur lexicale, ignoré.
