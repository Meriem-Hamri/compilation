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
static unsigned int current_column;
static unsigned int token_line = 1;
static unsigned int token_column = 1;
static unsigned int error_count = 0;

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
        ++current_line, current_column = 0;
    else if (c != EOF)
        ++current_column;

    return c;
}

static void unread_char(int c)
{
    if (c == EOF)
        return;

    if (c == '\n')
        --current_line, current_column = 0;
    else
        --current_column;

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

static void print_source_context(unsigned int line_number, unsigned int column)
{
    FILE *context_file;
    char line[512];
    char prefix[64];

    context_file = fopen(source_name, "r");
    if (context_file == NULL)
        return;

    for (unsigned int line_index = 1;
         line_index <= line_number && fgets(line, sizeof line, context_file) != NULL;
         ++line_index) {
        if (line_index == line_number) {
            line[strcspn(line, "\r\n")] = '\0';
            snprintf(prefix, sizeof prefix, "Ligne %u : ", line_number);
            fprintf(stderr, "    %s%s\n", prefix, line);
            fprintf(stderr, "    %*s^\n",
                    (int)strlen(prefix) + (int)column - 1, "");
            break;
        }
    }

    fclose(context_file);
}

static void lexical_error(int c)
{
    if (isprint((unsigned char)c))
        fprintf(stderr, "%s:%u: erreur: caractere inattendu '%c'\n",
                source_name, token_line, c);
    else
        fprintf(stderr, "%s:%u: erreur: caractere inattendu 0x%02X\n",
                source_name, token_line, (unsigned int)(unsigned char)c);

    print_source_context(token_line, token_column);
    ++error_count;
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

static token scanner(void)
{
    int in_char;
    int c;

    while ((in_char = read_char()) != EOF) {
        if (isspace((unsigned char)in_char))
            continue;

        clear_buffer();
        token_line = current_line;
        token_column = current_column;

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

int main(int argc, char *argv[])
{
    const char *output_name;
    FILE *output_file;
    token current_token;

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Utilisation: %s <fichier-source> [fichier-tokens]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    source_name = argv[1];
    output_name = argc == 3 ? argv[2] : "unites_lexicales.txt";

    source_file = fopen(source_name, "r");
    if (source_file == NULL) {
        fprintf(stderr, "%s: erreur: impossible d'ouvrir le fichier source\n",
                source_name);
        return EXIT_FAILURE;
    }

    output_file = fopen(output_name, "w");
    if (output_file == NULL) {
        fprintf(stderr, "%s: erreur: impossible de creer le fichier\n",
                output_name);
        fclose(source_file);
        return EXIT_FAILURE;
    }

    fprintf(output_file, "%-8s %-16s %s\n", "LIGNE", "UNITE", "LEXEME");
    fprintf(output_file, "%-8s %-16s %s\n", "-----", "-----", "------");

    while ((current_token = scanner()) != SCANEOF) {
        fprintf(output_file, "%-8u %-16s %s\n",
                token_line, token_name(current_token), token_buffer);
    }

    fclose(output_file);
    fclose(source_file);

    if (error_count != 0) {
        fprintf(stderr, "%u erreur(s) lexicale(s) detectee(s).\n", error_count);
        return EXIT_FAILURE;
    }

    printf("Analyse terminee: %s genere.\n", output_name);
    return EXIT_SUCCESS;
}
