# TP2 : analyse syntaxique avec recuperation des erreurs

Le TP2 lit le fichier de tokens produit par le TP1. Il affiche les erreurs
detectees et poursuit l'analyse au lieu de quitter au premier diagnostic.

## Compilation et lancement (depuis le dossier TP)

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic TP2/tp2.c -o TP2/tp2.exe
.\TP1\tp1.exe .\TP2\program.txt .\TP2\tokens.txt
if ($LASTEXITCODE -eq 0) {
    .\TP2\tp2.exe .\TP2\tokens.txt
}
```

## Principe de recuperation

- `syntax_error()` affiche le diagnostic et augmente un compteur.
- `match()` renvoie 0 en cas d'echec sans consommer le token incorrect.
- Les fonctions de reconnaissance peuvent transmettre cet echec a l'appelant.
- `recover_primary()` retrouve un operateur ou un separateur d'expression.
  Cela permet de signaler plusieurs operandes manquants dans un meme WRITE.
- Les listes READ reprennent a la virgule ou a leur fin.
- `recover_instruction()` retrouve un point-virgule, une instruction suivante,
  END ou la fin du fichier. Un ID suivi de := peut commencer une nouvelle
  affectation, meme sur la meme ligne ; `peek_token()` permet ce controle.
- Le lecteur signale aussi les lignes de tokens incorrectes et les ignore.

Le programme affiche un bilan et renvoie un code d'echec si au moins une erreur
a ete detectee. Il n'affiche le succes que si aucun compteur d'erreurs n'est actif.
Les erreurs d'ouverture ou de lecture du fichier restent des echecs d'entree-sortie.

L'exemple `program.txt` doit produire deux erreurs : l'operande absent apres
le moins de la ligne 3, puis l'operande absent apres le plus de la ligne 4.

La recuperation utilise des frontieres grammaticales pour limiter les diagnostics
en cascade. Les tokens sautes peuvent contenir d'autres fautes : comme toute
recuperation de ce type, elle ne garantit pas un diagnostic pour chaque faute
possible dans un fragment tres endommage.

## Verification

```powershell
python TP2/test_recovery.py
```

Le test compile les deux outils avec les avertissements traites comme des erreurs,
verifie les entrees valides et invalides, le nombre de diagnostics, les listes,
la reprise sur une meme ligne, les fichiers de tokens endommages et la terminaison.
