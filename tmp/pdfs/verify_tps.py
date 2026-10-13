from pathlib import Path
import subprocess, json

root = Path(__file__).resolve().parents[2]
work = root / 'tmp' / 'pdfs' / 'verification'
work.mkdir(parents=True, exist_ok=True)
for tp in ('TP1', 'TP2'):
    subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                    str(root/tp/(tp.lower()+'.c')), '-o', str(work/(tp.lower()+'.exe'))], check=True)

cases = {
 'exemple_tp1': (root/'TP1/program.txt').read_text(),
 'exemple_tp2': (root/'TP2/program.txt').read_text(),
 'reels': 'BEGIN WRITE(5.2, .5, 5., 0.25); END',
 'commentaire_eof': 'BEGIN WRITE(1); END -- sans retour final',
 'parentheses': 'BEGIN READ(x,y); x := ((x+2)-y); WRITE(x, 3.5); END',
 'operande_absent': 'BEGIN x := 10 + ; END',
 'signe_unaire': 'BEGIN x := -2.5; END',
 'scientifique': 'BEGIN x := 1e3; END',
 'double_point': 'BEGIN x := 5.2.3; END',
 'identifiant_chiffre': 'BEGIN x := 12abc; END',
 'read_vide': 'BEGIN READ(); END',
 'read_nombre': 'BEGIN READ(2); END',
 'virgule_finale': 'BEGIN READ(x,); END',
 'write_vide': 'BEGIN WRITE(); END',
 'programme_vide': 'BEGIN END',
 'apres_end': 'BEGIN WRITE(1); END WRITE(2);',
 'moins_commentaire': 'BEGIN x := 5--2;\nEND',
 'point_seul': 'BEGIN WRITE(.); END',
 'illegal': 'BEGIN WRITE(1@); END',
 'moins_avec_espace': 'BEGIN x := 5 - 2; END',
 'point_virgule_absent': 'BEGIN WRITE(1) END',
 'parenthese_absente': 'BEGIN WRITE((1+2); END',
 'fin_prematuree': 'BEGIN WRITE(1);',
 'casse': 'begin WRITE(1); END',
 'buffer_long': 'BEGIN '+ 'a'*256+' := 1; END',
}
results=[]
for name, text in cases.items():
    source=work/(name+'.micro'); tokens=work/(name+'.tokens')
    source.write_text(text, encoding='ascii')
    lex=subprocess.run([str(work/'tp1.exe'),str(source),str(tokens)], capture_output=True,text=True)
    syn=None
    if lex.returncode == 0:
        p=subprocess.run([str(work/'tp2.exe'),str(tokens)],capture_output=True,text=True)
        syn={'code':p.returncode,'message':p.stdout+p.stderr}
    results.append({'cas':name,'lexical':lex.returncode,'message_lexical':lex.stdout+lex.stderr,
                    'syntaxique':syn,'tokens':tokens.read_text() if tokens.exists() else ''})

stale=subprocess.run([str(work/'tp2.exe'),str(root/'TP1/unites_lexicales.txt')],capture_output=True,text=True)
results.append({'cas':'fichier_tokens_existant','syntaxique':{'code':stale.returncode,'message':stale.stdout+stale.stderr}})
(work/'resultats.json').write_text(json.dumps(results,ensure_ascii=False,indent=2),encoding='utf-8')
for r in results:
    print(r['cas'], 'LEX', r.get('lexical'), 'SYN', (r.get('syntaxique') or {}).get('code'))
    if r.get('syntaxique') and r['syntaxique']['code']:
        print(r['syntaxique']['message'].strip().replace(str(work),'<tests>'))

