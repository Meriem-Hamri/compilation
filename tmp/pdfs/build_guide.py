from pathlib import Path
import re, html, json, math
from reportlab.pdfgen import canvas
from reportlab.platypus import BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer, PageBreak, Table, TableStyle, Preformatted, KeepTogether, Flowable
from reportlab.platypus.tableofcontents import TableOfContents
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_LEFT
from reportlab.lib.colors import HexColor, white
from reportlab.lib.pagesizes import A4
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from pypdf import PdfReader

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'output/pdf'
OUT.mkdir(parents=True, exist_ok=True)
for name,file in [('Body','arial.ttf'),('Bold','arialbd.ttf'),('Italic','ariali.ttf'),('Mono','consola.ttf')]:
    pdfmetrics.registerFont(TTFont(name,'C:/Windows/Fonts/'+file))
pdfmetrics.registerFontFamily('Body',normal='Body',bold='Bold',italic='Italic',boldItalic='Bold')
NAVY=HexColor('#15334B'); TEAL=HexColor('#087E8B'); INK=HexColor('#253847'); LIGHT=HexColor('#EAF3F6'); GREY=HexColor('#61727F')
W,H=A4; WIDTH=W-96
styles={
 'body':ParagraphStyle('body',fontName='Body',fontSize=10.6,leading=15.1,textColor=INK,spaceAfter=8),
 'h1':ParagraphStyle('h1',fontName='Bold',fontSize=20,leading=24,textColor=NAVY,spaceAfter=15),
 'h2':ParagraphStyle('h2',fontName='Bold',fontSize=12.1,leading=16,textColor=TEAL,spaceBefore=8,spaceAfter=6,keepWithNext=True),
 'small':ParagraphStyle('small',fontName='Body',fontSize=9.1,leading=12.5,textColor=GREY,spaceAfter=7),
 'code':ParagraphStyle('code',fontName='Mono',fontSize=8.8,leading=12.1,textColor=NAVY,leftIndent=11,rightIndent=8,spaceBefore=4,spaceAfter=10,backColor=LIGHT,borderPadding=9),
 'cell':ParagraphStyle('cell',fontName='Body',fontSize=9.2,leading=12.2,textColor=INK),
 'th':ParagraphStyle('th',fontName='Bold',fontSize=9.2,leading=12.2,textColor=white),
 'bullet':ParagraphStyle('bullet',fontName='Body',fontSize=10.6,leading=15.1,textColor=INK,leftIndent=14,firstLineIndent=-11,spaceAfter=6),
 'cover':ParagraphStyle('cover',fontName='Bold',fontSize=34,leading=40,textColor=NAVY,spaceAfter=20),
 'subtitle':ParagraphStyle('subtitle',fontName='Body',fontSize=17,leading=25,textColor=TEAL,spaceAfter=22),
 'toc':ParagraphStyle('toc',fontName='Body',fontSize=9.4,leading=11.8,textColor=INK,spaceBefore=0,spaceAfter=0),
 'anncode':ParagraphStyle('anncode',fontName='Mono',fontSize=8,leading=11.1,textColor=INK,spaceAfter=0),
}

def rich(s):
    s=s.replace('\u2011','-').replace('\u2013','-').replace('\u2014','-')
    s=html.escape(s)
    s=re.sub(r'`([^`]+)`',lambda m:'<font name="Mono" size="9.7">'+m.group(1)+'</font>',s)
    s=re.sub(r'\*\*([^*]+)\*\*',r'<b>\1</b>',s)
    return s

class Pipeline(Flowable):
    def __init__(self): super().__init__(); self.width=WIDTH; self.height=114
    def draw(self):
        c=self.canv; bw=(WIDTH-30)/3
        for i,(title,lines) in enumerate([('TEXTE MICRO',['Des caractères','BEGIN x := 5.2; END']),('TP1 : LEXIQUE',['Des unités reconnues','ID, ASSIGNOP, REALLITERAL']),('TP2 : SYNTAXE',['Une structure vérifiée','Programme et instructions'])]):
            x=i*(bw+15); c.setFillColor(LIGHT); c.roundRect(x,17,bw,87,8,fill=1,stroke=0)
            c.setFillColor(TEAL); c.setFont('Bold',10.3); c.drawCentredString(x+bw/2,83,title)
            c.setFillColor(INK); c.setFont('Body',9)
            for j,line in enumerate(lines): c.drawCentredString(x+bw/2,61-j*17,line)
            if i<2:
                c.setStrokeColor(TEAL); c.setLineWidth(1.5); start=x+bw+3; end=x+bw+12
                c.line(start,60,end,60); c.line(end-3,63,end,60); c.line(end-3,57,end,60)

class GuideDoc(BaseDocTemplate):
    def __init__(self,path):
        super().__init__(str(path),pagesize=A4,leftMargin=48,rightMargin=48,topMargin=55,bottomMargin=48,
                         title='Comprendre la compilation - Cours, TP1 et TP2',author='Guide pédagogique préparé à partir des documents fournis',
                         subject='Analyse lexicale et analyse syntaxique de MICRO, explication progressive en français')
        self.addPageTemplates(PageTemplate(id='normal',frames=Frame(48,48,WIDTH,H-103,id='body',leftPadding=0,rightPadding=0,topPadding=0,bottomPadding=0),onPage=self.decorate))
        self.section=''; self.entries=[]
    def decorate(self,c,doc):
        if doc.page>1:
            c.setStrokeColor(HexColor('#D9E5EB')); c.setLineWidth(.7); c.line(48,H-35,W-48,H-35)
            c.setFillColor(GREY); c.setFont('Body',8); c.drawString(48,H-27,'COMPILATION  /  COURS + TP1 + TP2')
            c.drawRightString(W-48,H-27,'MICRO - ILISI')
        c.setStrokeColor(HexColor('#D9E5EB')); c.line(48,35,W-48,35)
        c.setFont('Body',8); c.setFillColor(GREY); c.drawString(48,23,'Guide progressif en français simple - 12 octobre 2026')
        c.drawRightString(W-48,23,str(doc.page))
    def afterFlowable(self,f):
        if isinstance(f,Paragraph) and getattr(f,'toc_key',None):
            txt=f.getPlainText(); key=f.toc_key
            self.canv.bookmarkPage(key); self.canv.addOutlineEntry(txt,key,0,False)
            if not key.startswith('ann_') or key.endswith('_0'): self.notify('TOCEntry',(0,txt,self.page,key))
            self.entries.append({'title':txt,'page':self.page,'key':key})

def table(lines,title):
    cells=[]
    for l in lines:
        safe=re.sub(r'`[^`]*`',lambda m:m.group(0).replace('|','\u0001'),l)
        parts=[x.strip().replace('\u0001','|') for x in safe.strip().strip('|').split('|')]
        if all(re.fullmatch(r'[:\- ]+',x) for x in parts): continue
        cells.append(parts)
    n=len(cells[0])
    assert all(len(row)==n for row in cells), (title,cells)
    if n==4 and title.startswith('37.'):
        widths=[32,39,150,WIDTH-221]
    elif n==4:
        widths=[112,108,110,WIDTH-330]
    elif title.startswith('42.'):
        widths=[WIDTH*.50,WIDTH*.13,WIDTH*.37]
    elif title.startswith('43.'):
        widths=[WIDTH*.29,WIDTH*.34,WIDTH*.37]
    elif n==3:
        widths=[WIDTH*.31,WIDTH*.31,WIDTH*.38]
    else:
        widths=[WIDTH/n]*n
    data=[[Paragraph(rich(x),styles['th' if j==0 else 'cell']) for x in row] for j,row in enumerate(cells)]
    t=Table(data,colWidths=widths,repeatRows=1,hAlign='LEFT')
    padding=3 if title.startswith('37.') else 5
    t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),NAVY),('VALIGN',(0,0),(-1,-1),'TOP'),
          ('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),('TOPPADDING',(0,0),(-1,-1),padding),('BOTTOMPADDING',(0,0),(-1,-1),padding),
          ('ROWBACKGROUNDS',(0,1),(-1,-1),[HexColor('#F1F6F8'),white]),('LINEBELOW',(0,0),(-1,0),.7,NAVY),
          ('LINEBELOW',(0,1),(-1,-1),.3,HexColor('#D8E4EA'))]))
    return [t,Spacer(1,10)]

def blocks(text,title):
    lines=text.strip().splitlines(); i=0; out=[]
    while i<len(lines):
        s=lines[i]
        if not s.strip(): i+=1; continue
        if s.startswith('### '):
            out.append(Paragraph(rich(s[4:]),styles['h2'])); i+=1
        elif s.startswith('```'):
            code=[]; i+=1
            while i<len(lines) and not lines[i].startswith('```'): code.append(lines[i]); i+=1
            i+=1
            out.append(Preformatted('\n'.join(code),styles['code']))
        elif s.startswith('|'):
            rows=[]
            while i<len(lines) and lines[i].startswith('|'): rows.append(lines[i]); i+=1
            out.extend(table(rows,title))
        elif s.startswith('- ') or re.match(r'^\d+\. ',s):
            if s.startswith('- '): value='• '+s[2:]
            else: value=s
            out.append(Paragraph(rich(value),styles['bullet'])); i+=1
        else:
            para=[s]; i+=1
            while i<len(lines) and lines[i].strip() and not lines[i].startswith(('### ','```','|','- ')) and not re.match(r'^\d+\. ',lines[i]):
                para.append(lines[i]); i+=1
            out.append(Paragraph(rich(' '.join(para)),styles['body']))
    return out

story=[]
story.append(Spacer(1,55))
story.append(Paragraph('COMPRENDRE<br/>LA COMPILATION',styles['cover']))
story.append(Paragraph('Le cours, puis les TP 1 et 2<br/>expliqués depuis le début',styles['subtitle']))
story.append(Paragraph('Un guide pour apprendre les notions pour la première fois, comprendre le code et savoir expliquer chaque choix.',styles['body']))
story.append(Spacer(1,24)); story.append(Pipeline()); story.append(Spacer(1,30))
story.append(Paragraph('Dans ce guide',styles['h2']))
for value in ['Des notions expliquées avec des exemples simples','Le scanner du TP1 et l’ajout des nombres réels','La grammaire MICRO et les fonctions du TP2','Des traces complètes, les erreurs réelles et leurs raisons','Des exercices corrigés et les deux codes C en annexe']:
    story.append(Paragraph('• '+rich(value),styles['bullet']))
story.append(Spacer(1,25)); story.append(Paragraph('ILISI - Langage MICRO<br/>Fondé sur le support de cours, les six photos et les fichiers C actuels.<br/>12 octobre 2026',styles['small']))
story.append(PageBreak())
story.append(Paragraph('Sommaire',styles['h1']))
story.append(Paragraph('Les numéros à gauche désignent les sections ; les numéros à droite indiquent les pages du présent guide. Les entrées sont cliquables dans le PDF.',styles['small']))
toc=TableOfContents(); toc.levelStyles=[styles['toc']]; story.append(toc)

raw=(ROOT/'tmp/pdfs/guide_content.md').read_text(encoding='utf-8')
sections=re.split(r'^@page ',raw,flags=re.M)[1:]
for k,section in enumerate(sections):
    title,body=section.split('\n',1)
    story.append(PageBreak()); h=Paragraph(rich(title),styles['h1']); h.toc_key='section_'+str(k); story.append(h)
    story.extend(blocks(body,title))

for tp,annname in [('TP1','Annexe A - Code complet du TP1'),('TP2','Annexe B - Code complet du TP2')]:
    path=ROOT/tp/(tp.lower()+'.c'); source=path.read_text(encoding='utf-8').splitlines()
    # Leave room for headers and preserve the exact source, including empty lines.
    chunk_size=math.ceil(len(source)/math.ceil(len(source)/53))
    chunks=[source[i:i+chunk_size] for i in range(0,len(source),chunk_size)]
    for idx,chunk in enumerate(chunks):
        story.append(PageBreak()); label=annname if idx==0 else annname+' - suite '+str(idx)
        h=Paragraph(label,styles['h1']); h.toc_key='ann_'+tp+'_'+str(idx); story.append(h)
        first=idx*chunk_size+1; last=first+len(chunk)-1
        story.append(Paragraph(f'Fichier {tp}/{tp.lower()}.c - lignes {first} à {last}. Les numéros à gauche sont des repères de lecture ; ils ne font pas partie du code.',styles['small']))
        numbered='\n'.join(f'{first+i:03d}  {line}' for i,line in enumerate(chunk))
        story.append(Preformatted(numbered,styles['anncode']))

path=OUT/'Compilation_Cours_TP1_TP2_Guide_detaille.pdf'
doc=GuideDoc(path); doc.multiBuild(story)
reader=PdfReader(path)
(ROOT/'tmp/pdfs/structure.json').write_text(json.dumps({'pages':len(reader.pages),'sections':doc.entries},ensure_ascii=False,indent=2),encoding='utf-8')
print('PDF:',path)
print('PAGES:',len(reader.pages))
print('WORDS:',sum(len((p.extract_text() or '').split()) for p in reader.pages))
for i,p in enumerate(reader.pages):
    text=p.extract_text() or ''
    if len(text.split())<35: print('PAGE PEU REMPLIE:',i+1,len(text.split()))

