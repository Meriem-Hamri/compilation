#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_BUFFER_SIZE 256

typedef enum {
    BEGIN_TOKEN,
    END_TOKEN,
    READ_TOKEN,
    WRITE_TOKEN,
    ID,
    INTLITERAL,
    REALLITERAL,
    LPAREN,
    RPAREN,
    SEMICOLON,
    COMMA,
    ASSIGNOP,
    PLUSOP,
    MINUSOP,
    SCANEOF
} token;

static FILE *source_file;
static const char *source_name;
static char token_buffer[TOKEN_BUFFER_SIZE];
static unsigned int token_line = 1;
static unsigned int last_token_line = 1;
static token current_token;
static unsigned int syntax_error_count;
static unsigned int input_error_count;
static unsigned int instruction_line;
static int has_lookahead;
static token lookahead_token;
static char lookahead_buffer[TOKEN_BUFFER_SIZE];
static unsigned int lookahead_line;

static const char *token_description(token value)
{
    switch (value) {
        case BEGIN_TOKEN: return "'BEGIN'";
        case END_TOKEN:   return "'END'";
        case READ_TOKEN:  return "'READ'";
        case WRITE_TOKEN: return "'WRITE'";
        case ID:           return "un identificateur";
        case INTLITERAL:   return "un entier";
        case REALLITERAL:  return "un reel";
        case LPAREN:       return "'('";
        case RPAREN:       return "')'";
        case SEMICOLON:    return "';'";
        case COMMA:        return "','";
        case ASSIGNOP:     return "':='";
        case PLUSOP:       return "'+'";
        case MINUSOP:      return "'-'";
        case SCANEOF:      return "la fin du fichier";
    }

    return "un symbole inconnu";
}

static token token_from_name(const char *name)
{
    static const char *const names[] = {
        "BEGIN", "END", "READ", "WRITE", "ID", "INTLITERAL",
        "REALLITERAL", "LPAREN", "RPAREN", "SEMICOLON", "COMMA",
        "ASSIGNOP", "PLUSOP", "MINUSOP"
    };
    size_t index;

    for (index = 0; index < sizeof names / sizeof names[0]; ++index) {
        if (strcmp(name, names[index]) == 0)
            return (token)index;
    }

    return SCANEOF;
}

/* Lit une ligne produite par le TP1 et restitue le token correspondant. */
static token read_token(void)
{
    char line[512];
    char token_name[32];
    token value;
    unsigned int line_number;

    while (fgets(line, sizeof line, source_file) != NULL) {
        if (strncmp(line, "LIGNE", 5) == 0 || strncmp(line, "-----", 5) == 0)
            continue;

        if (sscanf(line, "%u %31s %255[^\n]", &line_number,
                   token_name, token_buffer) != 3) {
            fprintf(stderr, "Erreur : ligne mal ecrite dans le fichier de tokens : %s",
                    line);
            ++input_error_count;
            continue;
        }

        value = token_from_name(token_name);
        if (value == SCANEOF) {
            fprintf(stderr, "Erreur ligne %u : token '%s' inconnu.\n",
                    line_number, token_name);
            ++input_error_count;
            continue;
        }

        token_line = line_number;
        last_token_line = line_number;
        return value;
    }

    token_buffer[0] = '\0';
    token_line = last_token_line;
    return SCANEOF;
}

static token scanner(void)
{
    if (has_lookahead) {
        has_lookahead = 0;
        memcpy(token_buffer, lookahead_buffer, sizeof token_buffer);
        token_line = lookahead_line;
        last_token_line = lookahead_line;
        return lookahead_token;
    }
    return read_token();
}

/* Seulement pour la recuperation : distinguer une affectation de la fin
 * d'une expression incorrecte, sans perdre le lexeme du token courant.
 */
static token peek_token(void)
{
    if (!has_lookahead) {
        char saved_buffer[TOKEN_BUFFER_SIZE];
        unsigned int saved_line = token_line;
        unsigned int saved_last_line = last_token_line;

        memcpy(saved_buffer, token_buffer, sizeof saved_buffer);
        lookahead_token = read_token();
        memcpy(lookahead_buffer, token_buffer, sizeof lookahead_buffer);
        lookahead_line = token_line;
        has_lookahead = 1;
        memcpy(token_buffer, saved_buffer, sizeof token_buffer);
        token_line = saved_line;
        last_token_line = saved_last_line;
    }
    return lookahead_token;
}

/* Un token d'anticipation : consulter ne consomme jamais le token. */
static token next_token(void)
{
    return current_token;
}

static void syntax_error(const char *expected)
{
    fprintf(stderr, "Erreur ligne %u : %s", token_line, expected);
    if (next_token() == SCANEOF)
        fputs(" (fin du fichier).", stderr);
    else
        fprintf(stderr, " (trouve : '%s').", token_buffer);
    fputc('\n', stderr);
    ++syntax_error_count;
}

/* Un echec laisse le token courant disponible pour la recuperation. */
static int match(token expected)
{
    if (next_token() != expected) {
        char message[128];
        switch (expected) {
            case BEGIN_TOKEN:
                snprintf(message, sizeof message, "le programme doit commencer par BEGIN");
                break;
            case END_TOKEN:
                snprintf(message, sizeof message, "il manque END a la fin du programme");
                break;
            case SEMICOLON:
                snprintf(message, sizeof message, "il manque ';' a la fin de l'instruction");
                break;
            case LPAREN:
                snprintf(message, sizeof message, "il manque '('");
                break;
            case RPAREN:
                snprintf(message, sizeof message, "il manque ')'");
                break;
            case ASSIGNOP:
                snprintf(message, sizeof message, "il manque ':=' apres le nom de variable");
                break;
            case ID:
                snprintf(message, sizeof message, "il faut un nom de variable ici");
                break;
            case SCANEOF:
                snprintf(message, sizeof message, "il y a du texte apres END");
                break;
            default:
                snprintf(message, sizeof message, "on attend %s", token_description(expected));
                break;
        }
        syntax_error(message);
        return 0;
    }

    if (expected != SCANEOF)
        current_token = scanner();
    return 1;
}

static int expression(void);
static int inst(void);

/* Une erreur locale ne doit pas faire perdre les autres elements de la liste. */
static int expression_boundary(token tok)
{
    return tok == COMMA || tok == RPAREN || tok == SEMICOLON ||
           tok == END_TOKEN || tok == SCANEOF || tok == READ_TOKEN ||
           tok == WRITE_TOKEN || tok == BEGIN_TOKEN;
}

static void recover_primary(void)
{
    while (!expression_boundary(next_token()) &&
           next_token() != PLUSOP && next_token() != MINUSOP)
        current_token = scanner();
}

/* <add_op> ::= PLUSOP | MINUSOP */
static void add_op(void)
{
    token tok = next_token();
    if (tok == PLUSOP || tok == MINUSOP)
        match(tok);
    else
        syntax_error("il faut '+' ou '-' ici");
}

/* <prim> ::= '(' <expr> ')' | ID | INTLITERAL | REALLITERAL */
static int prim(const char *expected)
{
    switch (next_token()) {
        case LPAREN:
            match(LPAREN);
            if (!expression() || !match(RPAREN))
                return 0;
            break;
        case ID:
            match(ID);
            break;
        case INTLITERAL:
            match(INTLITERAL);
            break;
        case REALLITERAL:
            match(REALLITERAL);
            break;
        default:
            syntax_error(expected);
            return 0;
    }
    return 1;
}

/* <expr> ::= <prim> { <add_op> <prim> } */
static int expression(void)
{
    if (!prim("il faut une variable, un nombre ou une expression entre parentheses"))
        recover_primary();
    while (next_token() == PLUSOP || next_token() == MINUSOP) {
        char message[128];
        token op = next_token();
        snprintf(message, sizeof message, "il manque une valeur apres %s",
                 token_description(op));
        add_op();
        if (!prim(message))
            recover_primary();
    }
    return 1;
}

/* <id_list> ::= ID { ',' ID } : au moins un identificateur. */
static int id_list(void)
{
    if (!match(ID)) {
        while (!expression_boundary(next_token()))
            current_token = scanner();
    }
    while (next_token() == COMMA) {
        match(COMMA);
        if (!match(ID)) {
            while (!expression_boundary(next_token()))
                current_token = scanner();
        }
    }
    return 1;
}

/* <expr_list> ::= <expr> { ',' <expr> } : au moins une expression. */
static int expr_list(void)
{
    if (!expression())
        return 0;
    while (next_token() == COMMA) {
        match(COMMA);
        if (!expression())
            return 0;
    }
    return 1;
}

/* <inst> ::= ID ':=' <expr> ';'
 *          | READ '(' <id_list> ')' ';'
 *          | WRITE '(' <expr_list> ')' ';'
 */
static int inst(void)
{
    instruction_line = token_line;
    switch (next_token()) {
        case ID:
            match(ID);
            if (!match(ASSIGNOP) || !expression() || !match(SEMICOLON))
                return 0;
            break;
        case READ_TOKEN:
            match(READ_TOKEN);
            if (!match(LPAREN) || !id_list() || !match(RPAREN) ||
                !match(SEMICOLON))
                return 0;
            break;
        case WRITE_TOKEN:
            match(WRITE_TOKEN);
            if (!match(LPAREN) || !expr_list() || !match(RPAREN) ||
                !match(SEMICOLON))
                return 0;
            break;
        default:
            syntax_error("il faut une affectation, READ ou WRITE ici");
            return 0;
    }
    return 1;
}

static int starts_instruction(token tok)
{
    return tok == ID || tok == READ_TOKEN || tok == WRITE_TOKEN;
}

/* Mode panique : retrouver une frontiere pour reprendre la grammaire.
 * END, EOF et le debut de l'instruction suivante ne sont pas consommes.
 * Le point-virgule termine l'instruction incorrecte et est consomme.
 */
static void recover_instruction(void)
{
    while (next_token() != SEMICOLON && next_token() != END_TOKEN &&
           next_token() != SCANEOF && next_token() != READ_TOKEN &&
           next_token() != WRITE_TOKEN &&
           !(next_token() == ID && (token_line > instruction_line ||
                                   peek_token() == ASSIGNOP)))
        current_token = scanner();

    if (next_token() == SEMICOLON)
        current_token = scanner();
}

/* <inst_list> ::= <inst> { <inst> } : au moins une instruction. */
static void inst_list(void)
{
    if (next_token() == END_TOKEN || next_token() == SCANEOF) {
        syntax_error("il faut au moins une instruction entre BEGIN et END");
        return;
    }

    while (next_token() != END_TOKEN && next_token() != SCANEOF) {
        if (starts_instruction(next_token())) {
            /* inst() consomme toujours son premier token : progres garanti. */
            if (!inst())
                recover_instruction();
        } else {
            syntax_error("il faut une affectation, READ ou WRITE ici");
            current_token = scanner();
            recover_instruction();
        }
    }
}

/* <program> ::= BEGIN <inst_list> END */
static void program(void)
{
    match(BEGIN_TOKEN);
    inst_list();
    match(END_TOKEN);
}

/* <system_goal> ::= <program> SCANEOF */
static void system_goal(void)
{
    program();
    if (!match(SCANEOF)) {
        /* Lire aussi les lignes de tokens restantes pour signaler leur format. */
        while (next_token() != SCANEOF)
            current_token = scanner();
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Utilisation: %s <fichier-tokens>\n", argv[0]);
        return EXIT_FAILURE;
    }

    source_name = argv[1];
    source_file = fopen(source_name, "r");
    if (source_file == NULL) {
        fprintf(stderr, "%s: erreur: impossible d'ouvrir le fichier source\n",
                source_name);
        return EXIT_FAILURE;
    }

    current_token = scanner();
    system_goal();
    if (ferror(source_file)) {
        fprintf(stderr, "%s: erreur de lecture du fichier source\n", source_name);
        fclose(source_file);
        return EXIT_FAILURE;
    }
    fclose(source_file);
    if (syntax_error_count != 0 || input_error_count != 0) {
        fprintf(stderr, "Total : %u erreur(s) de syntaxe.\n", syntax_error_count);
        if (input_error_count != 0)
            fprintf(stderr, "Erreurs dans le fichier de tokens : %u.\n", input_error_count);
        return EXIT_FAILURE;
    }
    puts("Pas d'erreur de syntaxe : programme valide.");
    return EXIT_SUCCESS;
}
