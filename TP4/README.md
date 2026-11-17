# TP4 - Implémentation d'automates (analyseur lexical)

Binôme : HAMRI Meliem, BAKKAT Zakaria

Unités : `si`, `alors`, `sinon`, `oprel`, `id`, `nb` (mots-clés réservés).
Compilation / test : `gcc -Wall -o tp4 tp4.c && ./tp4 test.txt`

## Étape 1 : AFD (version corrigée)

Notation : (n) état, [n] état final, `ch` = chiffre, `l` = lettre.

1. **si** : (0) -s-> (1) -i-> [2]
2. **alors** : (3) -a-> (4) -l-> (5) -o-> (6) -r-> (7) -s-> [8]
3. **sinon** : (9) -s-> (10) -i-> (11) -n-> (12) -o-> (13) -n-> [14]

   Un mot-clé n'est accepté que si le caractère suivant n'est ni lettre ni
   chiffre (sinon `sinonx`, `si1` sont des `id`).
4. **oprel** :
   (15) -<-> [16], (15) -> -> [17], (15) -=-> [18],
   [16] -(= | >)-> [18], [17] -=-> [18]
5. **id** : (19) -l-> [20], [20] -(l | ch)-> [20]
6. **nb** : `ch+ (. ch+)? (E (+|-)? ch+)?`

| état | ch | . | E | + / - | final |
|------|----|---|---|-------|-------|
| 21   | 22 |   |   |       | non   |
| 22   | 22 | 23| 25|       | oui   |
| 23   | 24 |   |   |       | non   |
| 24   | 24 |   | 25|       | oui   |
| 25   | 27 |   |   | 26    | non   |
| 26   | 27 |   |   |       | non   |
| 27   | 27 |   |   |       | oui   |

Corrections par rapport au premier schéma manuscrit :
- état 26 (après le signe) : sur `chiffre` on va à **27** (il n'a pas de boucle et n'est pas final) ;
- la transition 24 -> 25 est étiquetée **E** ;
- règle de délimiteur pour les mots-clés (voir 3).

## Étape 2 : implémentation

Chaque AFD est une fonction C (`switch/case` sur l'état) qui retient le plus long
préfixe reconnu. `Echec()` remet le pointeur au début du lexème et passe à
l'AFD suivant (ordre : si, alors, sinon, oprel, nb, id). Si tous échouent :
erreur lexicale (caractère signalé avec sa colonne, puis on continue).
