#include <ctype.h>
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
static size_t token_buffer_index;
static unsigned int current_line = 1;
static unsigned int token_line = 1;
static token current_token;

static void clear_buffer(void)
{
    token_buffer_index = 0;
    token_buffer[0] = '\0';
}

static void buffer_char(int c)
{
    if (token_buffer_index + 1 >= TOKEN_BUFFER_SIZE) {
        fprintf(stderr, "%s:%u: erreur: unite lexicale trop longue\n",
                source_name, token_line);
        exit(EXIT_FAILURE);
    }

    token_buffer[token_buffer_index++] = (char)c;
    token_buffer[token_buffer_index] = '\0';
}

static int read_char(void)
{
    int c = fgetc(source_file);

    if (c == '\n')
        ++current_line;

    return c;
}

static void unread_char(int c)
{
    if (c == EOF)
        return;

    if (c == '\n')
        --current_line;

    ungetc(c, source_file);
}

static token check_reserved(void)
{
    if (strcmp(token_buffer, "BEGIN") == 0)
        return BEGIN_TOKEN;
    if (strcmp(token_buffer, "END") == 0)
        return END_TOKEN;
    if (strcmp(token_buffer, "READ") == 0)
        return READ_TOKEN;
    if (strcmp(token_buffer, "WRITE") == 0)
        return WRITE_TOKEN;

    return ID;
}

static void lexical_error(int c)
{
    if (isprint((unsigned char)c))
        fprintf(stderr, "%s:%u: erreur: caractere inattendu '%c'\n",
                source_name, token_line, c);
    else
        fprintf(stderr, "%s:%u: erreur: caractere inattendu 0x%02X\n",
                source_name, token_line, (unsigned int)(unsigned char)c);

    fclose(source_file);
    exit(EXIT_FAILURE);
}

static const char *token_name(token value)
{
    switch (value) {
        case BEGIN_TOKEN: return "BEGIN";
        case END_TOKEN:   return "END";
        case READ_TOKEN:  return "READ";
        case WRITE_TOKEN: return "WRITE";
        case ID:           return "ID";
        case INTLITERAL:   return "INTLITERAL";
        case REALLITERAL:  return "REALLITERAL";
        case LPAREN:       return "LPAREN";
        case RPAREN:       return "RPAREN";
        case SEMICOLON:    return "SEMICOLON";
        case COMMA:        return "COMMA";
        case ASSIGNOP:     return "ASSIGNOP";
        case PLUSOP:       return "PLUSOP";
        case MINUSOP:      return "MINUSOP";
        case SCANEOF:      return "SCANEOF";
    }

    return "UNKNOWN";
}

/* Scanner repris du TP1, avec arret immediat sur erreur lexicale. */
static token scanner(void)
{
    int in_char;
    int c;

    while ((in_char = read_char()) != EOF) {
        if (isspace((unsigned char)in_char))
            continue;

        clear_buffer();
        token_line = current_line;

        if (isalpha((unsigned char)in_char)) {
            buffer_char(in_char);

            for (c = read_char();
                 c != EOF && (isalnum((unsigned char)c) || c == '_');
                 c = read_char())
                buffer_char(c);

            unread_char(c);
            return check_reserved();
        }

        if (isdigit((unsigned char)in_char) || in_char == '.') {
            int is_real = in_char == '.';

            buffer_char(in_char);

            if (in_char == '.') {
                c = read_char();
                if (c == EOF || !isdigit((unsigned char)c)) {
                    unread_char(c);
                    lexical_error(in_char);
                    continue;
                }
                buffer_char(c);
            }

            for (c = read_char();
                 c != EOF && isdigit((unsigned char)c);
                 c = read_char())
                buffer_char(c);

            if (!is_real && c == '.') {
                is_real = 1;
                buffer_char(c);
                for (c = read_char();
                     c != EOF && isdigit((unsigned char)c);
                     c = read_char())
                    buffer_char(c);
            }

            unread_char(c);
            return is_real ? REALLITERAL : INTLITERAL;
        }

        buffer_char(in_char);

        switch (in_char) {
            case '(':
                return LPAREN;
            case ')':
                return RPAREN;
            case ';':
                return SEMICOLON;
            case ',':
                return COMMA;
            case '+':
                return PLUSOP;

            case ':':
                c = read_char();
                if (c == '=') {
                    buffer_char(c);
                    return ASSIGNOP;
                }
                unread_char(c);
                lexical_error(in_char);
                break;

            case '-':
                c = read_char();
                if (c == '-') {
                    do {
                        in_char = read_char();
                    } while (in_char != '\n' && in_char != EOF);
                } else {
                    unread_char(c);
                    return MINUSOP;
                }
                break;

            default:
                lexical_error(in_char);
                break;
        }
    }

    clear_buffer();
    token_line = current_line;
    return SCANEOF;
}

/* Un token d'anticipation : consulter ne consomme jamais le token. */
static token next_token(void)
{
    return current_token;
}

static void syntax_error(const char *expected)
{
    fprintf(stderr, "%s:%u: erreur syntaxique: %s attendu, %s rencontre",
            source_name, token_line, expected, token_name(next_token()));
    if (next_token() != SCANEOF)
        fprintf(stderr, " ('%s')", token_buffer);
    fputc('\n', stderr);
    fclose(source_file);
    exit(EXIT_FAILURE);
}

static void match(token expected)
{
    if (next_token() != expected)
        syntax_error(token_name(expected));

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
        syntax_error("PLUSOP ou MINUSOP");
}

/* <prim> ::= '(' <expr> ')' | ID | INTLITERAL | REALLITERAL */
static void prim(void)
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
            syntax_error("ID, INTLITERAL, REALLITERAL ou LPAREN");
            break;
    }
}

/* <expr> ::= <prim> { <add_op> <prim> } */
static void expression(void)
{
    prim();
    while (next_token() == PLUSOP || next_token() == MINUSOP) {
        add_op();
        prim();
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
            syntax_error("instruction (ID, READ ou WRITE)");
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
        fprintf(stderr, "Utilisation: %s <fichier-source>\n", argv[0]);
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


