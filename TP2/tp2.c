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
static token scanner(void)
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
            fprintf(stderr, "%s: erreur: ligne de tokens invalide: %s",
                    source_name, line);
            fclose(source_file);
            exit(EXIT_FAILURE);
        }

        value = token_from_name(token_name);
        if (value == SCANEOF) {
            fprintf(stderr, "%s:%u: erreur: unite inconnue '%s'\n",
                    source_name, line_number, token_name);
            fclose(source_file);
            exit(EXIT_FAILURE);
        }

        token_line = line_number;
        last_token_line = line_number;
        return value;
    }

    token_buffer[0] = '\0';
    token_line = last_token_line;
    return SCANEOF;
}

/* Un token d'anticipation : consulter ne consomme jamais le token. */
static token next_token(void)
{
    return current_token;
}

static void syntax_error(const char *expected)
{
    fprintf(stderr, "%s:%u: erreur syntaxique : %s ; ",
            source_name, token_line, expected);
    if (next_token() == SCANEOF)
        fputs("fin du fichier rencontree", stderr);
    else
        fprintf(stderr, "'%s' rencontre", token_buffer);
    fputc('\n', stderr);
    fclose(source_file);
    exit(EXIT_FAILURE);
}

static void match(token expected)
{
    if (next_token() != expected) {
        char message[128];
        if (expected == SCANEOF)
            snprintf(message, sizeof message, "fin du fichier attendue");
        else
            snprintf(message, sizeof message, "%s attendu", token_description(expected));
        syntax_error(message);
    }

    if (expected != SCANEOF)
        current_token = scanner();
}

static void expression(void);
static void inst(void);

/* <add_op> ::= PLUSOP | MINUSOP */
static void add_op(void)
{
    token tok = next_token();
    if (tok == PLUSOP || tok == MINUSOP)
        match(tok);
    else
        syntax_error("operateur '+' ou '-' attendu");
}

/* <prim> ::= '(' <expr> ')' | ID | INTLITERAL | REALLITERAL */
static void prim(const char *expected)
{
    switch (next_token()) {
        case LPAREN:
            match(LPAREN);
            expression();
            match(RPAREN);
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
            break;
    }
}

/* <expr> ::= <prim> { <add_op> <prim> } */
static void expression(void)
{
    prim("identificateur, nombre ou expression entre parentheses attendu");
    while (next_token() == PLUSOP || next_token() == MINUSOP) {
        char message[128];
        token op = next_token();
        snprintf(message, sizeof message, "operande attendu apres %s",
                 token_description(op));
        add_op();
        prim(message);
    }
}

/* <id_list> ::= ID { ',' ID } : au moins un identificateur. */
static void id_list(void)
{
    match(ID);
    while (next_token() == COMMA) {
        match(COMMA);
        match(ID);
    }
}

/* <expr_list> ::= <expr> { ',' <expr> } : au moins une expression. */
static void expr_list(void)
{
    expression();
    while (next_token() == COMMA) {
        match(COMMA);
        expression();
    }
}

/* <inst> ::= ID ':=' <expr> ';'
 *          | READ '(' <id_list> ')' ';'
 *          | WRITE '(' <expr_list> ')' ';'
 */
static void inst(void)
{
    switch (next_token()) {
        case ID:
            match(ID);
            match(ASSIGNOP);
            expression();
            match(SEMICOLON);
            break;
        case READ_TOKEN:
            match(READ_TOKEN);
            match(LPAREN);
            id_list();
            match(RPAREN);
            match(SEMICOLON);
            break;
        case WRITE_TOKEN:
            match(WRITE_TOKEN);
            match(LPAREN);
            expr_list();
            match(RPAREN);
            match(SEMICOLON);
            break;
        default:
            syntax_error("instruction attendue (affectation, READ ou WRITE)");
            break;
    }
}

/* <inst_list> ::= <inst> { <inst> } : au moins une instruction. */
static void inst_list(void)
{
    inst();
    while (next_token() == ID || next_token() == READ_TOKEN ||
           next_token() == WRITE_TOKEN)
        inst();
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
    match(SCANEOF);
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
    puts("Analyse syntaxique reussie: programme valide.");
    return EXIT_SUCCESS;
}
