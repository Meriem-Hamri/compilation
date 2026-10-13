"""Tests de l'analyse syntaxique et de sa reprise apres erreur.

Lancer depuis n'importe quel dossier : python TP2/test_recovery.py
GCC doit etre disponible. Les fichiers de test restent dans un dossier temporaire.
"""
from pathlib import Path
import random
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def run(command):
    return subprocess.run(command, capture_output=True, text=True, timeout=5)


def main():
    with tempfile.TemporaryDirectory(prefix="micro_recovery_") as folder:
        work = Path(folder)
        lexer = work / "tp1.exe"
        parser = work / "tp2.exe"
        for name, target in (("TP1", lexer), ("TP2", parser)):
            subprocess.run([
                "gcc", "-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                str(ROOT / name / (name.lower() + ".c")), "-o", str(target)
            ], check=True)

        cases = [
            ("programme_valide", "BEGIN READ(x,y); x := ((x+2)-y); WRITE(x,.5,5.); END", 0),
            ("exemple_du_tp2", (ROOT / "TP2/program.txt").read_text(), 2),
            ("deux_erreurs_meme_write", "BEGIN WRITE(x + , y + ); END", 2),
            ("trois_erreurs_meme_read", "BEGIN READ(2,3,x,); END", 3),
            ("instructions_meme_ligne", "BEGIN x := 1 y := ; WRITE(2 + ); END", 3),
            ("instructions_lignes_distinctes", "BEGIN\nx := 1\ny := ;\nWRITE(2 + );\nEND", 3),
            ("parentheses_et_listes", "BEGIN WRITE((x + ), (y - )); END", 2),
            ("instruction_inconnue", "BEGIN ; + WRITE(1 + ); x := ; END", 3),
            ("notation_scientifique_sans_cascade", "BEGIN x := 1e3; WRITE(2+); END", 2),
            ("identificateur_sans_cascade", "BEGIN x := 12abc; WRITE(2+); END", 2),
            ("fin_prematuree", "BEGIN x :=", 3),
            ("programme_vide", "BEGIN END", 1),
            ("texte_apres_end", "BEGIN WRITE(1); END WRITE(2);", 1),
            ("commentaire_final", "BEGIN WRITE(1); END -- fin", 0),
            ("parenthese_absente_et_autre_erreur", "BEGIN WRITE((x+2); y := ; END", 2),
        ]
        for name, source, errors in cases:
            micro = work / (name + ".micro")
            tokens = work / (name + ".tokens")
            micro.write_text(source, encoding="ascii")
            lexical = run([str(lexer), str(micro), str(tokens)])
            assert lexical.returncode == 0, (name, lexical.stderr)
            result = run([str(parser), str(tokens)])
            assert result.returncode == (1 if errors else 0), (name, result.stderr)
            assert result.stderr.count("Erreur ligne ") == errors, (name, result.stderr)
            if errors:
                assert f"Total : {errors} erreur(s) de syntaxe." in result.stderr, (name, result.stderr)
                assert "programme valide" not in result.stdout, (name, result.stdout)
            else:
                assert "programme valide" in result.stdout, (name, result.stdout)

        # Les lignes de tokens incorrectes sont toutes signalees, puis sautees.
        damaged = work / "damaged.tokens"
        damaged.write_text(
            "LIGNE UNITE LEXEME\n----- ----- ------\n"
            "1 BEGIN BEGIN\nligne invalide\n1 INCONNU ?\n"
            "1 WRITE WRITE\n1 LPAREN (\n1 INTLITERAL 1\n"
            "1 RPAREN )\n1 SEMICOLON ;\n1 END END\n", encoding="ascii"
        )
        result = run([str(parser), str(damaged)])
        assert result.returncode == 1, result.stdout
        assert "Total : 0 erreur(s) de syntaxe." in result.stderr, result.stderr
        assert "Erreurs dans le fichier de tokens : 2." in result.stderr, result.stderr

        # Des suites arbitraires doivent toujours terminer, meme sans END ni ';'.
        rng = random.Random(13)
        categories = ["BEGIN", "END", "READ", "WRITE", "ID", "INTLITERAL",
                      "REALLITERAL", "LPAREN", "RPAREN", "SEMICOLON", "COMMA",
                      "ASSIGNOP", "PLUSOP", "MINUSOP"]
        for index in range(100):
            tokens = work / f"fuzz_{index}.tokens"
            tokens.write_text("".join(
                f"{position // 8 + 1} {rng.choice(categories)} texte\n"
                for position in range(rng.randrange(0, 60))
            ), encoding="ascii")
            result = run([str(parser), str(tokens)])
            assert result.returncode in (0, 1), (index, result.stderr)

    print("OK : 15 cas grammaticaux, fichier de tokens endommage et 100 suites arbitraires.")


if __name__ == "__main__":
    main()
