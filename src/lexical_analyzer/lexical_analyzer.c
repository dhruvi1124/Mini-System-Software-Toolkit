#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexical_analyzer.h"
#include "../common/common.h"

/* Complete set of 44 standard C17 keywords */
static const char *C17_KEYWORDS[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "inline", "int", "long", "register", "restrict", "return", "short",
    "signed", "sizeof", "static", "struct", "switch", "typedef", "union",
    "unsigned", "void", "volatile", "while",
    "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic",
    "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
};
#define NUM_KEYWORDS (sizeof(C17_KEYWORDS) / sizeof(C17_KEYWORDS[0]))

int is_keyword(const char *word) {
    if (!word || *word == '\0') return 0;
    for (size_t i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(word, C17_KEYWORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

const char *token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_KEYWORD:                return "KEYWORD";
        case TOKEN_IDENTIFIER:             return "IDENTIFIER";
        case TOKEN_INTEGER_CONSTANT:       return "INTEGER_CONSTANT";
        case TOKEN_FLOAT_CONSTANT:         return "FLOAT_CONSTANT";
        case TOKEN_CHARACTER_CONSTANT:     return "CHARACTER_CONSTANT";
        case TOKEN_STRING_LITERAL:         return "STRING_LITERAL";
        case TOKEN_OPERATOR:               return "OPERATOR";
        case TOKEN_SEPARATOR:              return "SEPARATOR";
        case TOKEN_PARENTHESIS:            return "PARENTHESIS";
        case TOKEN_COMMENT:                return "COMMENT";
        case TOKEN_PREPROCESSOR_DIRECTIVE: return "PREPROCESSOR_DIRECTIVE";
        case TOKEN_UNKNOWN:                return "UNKNOWN";
        default:                           return "UNKNOWN";
    }
}

static void format_lexeme_display(const char *src, char *dest, size_t dest_size) {
    size_t d = 0;
    for (size_t s = 0; src[s] != '\0' && d + 2 < dest_size; s++) {
        if (src[s] == '\n') {
            dest[d++] = '\\';
            dest[d++] = 'n';
        } else if (src[s] == '\r') {
            dest[d++] = '\\';
            dest[d++] = 'r';
        } else if (src[s] == '\t') {
            dest[d++] = '\\';
            dest[d++] = 't';
        } else {
            dest[d++] = src[s];
        }
    }
    dest[d] = '\0';
}

int tokenize_source(const char *source, Token *tokens, int max_tokens) {
    if (!source || !tokens || max_tokens <= 0) return 0;

    int i = 0;
    int token_count = 0;
    int line_number = 1;

    while (source[i] != '\0' && token_count < max_tokens) {
        char c = source[i];

        /* Handle newlines for line tracking */
        if (c == '\n') {
            line_number++;
            i++;
            continue;
        }

        /* Skip other whitespace */
        if (isspace((unsigned char)c)) {
            i++;
            continue;
        }

        /* 1. Preprocessor directives (#include, #define, etc.) */
        if (c == '#') {
            int start = i;
            i++; /* skip '#' */
            while (source[i] != '\0' && source[i] != '\n') {
                if (source[i] == '\\' && source[i + 1] == '\n') {
                    i += 2;
                    line_number++;
                } else {
                    i++;
                }
            }
            int len = i - start;
            if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
            strncpy(tokens[token_count].lexeme, &source[start], len);
            tokens[token_count].lexeme[len] = '\0';
            tokens[token_count].type = TOKEN_PREPROCESSOR_DIRECTIVE;
            tokens[token_count].token_number = token_count + 1;
            tokens[token_count].line_number = line_number;
            token_count++;
            continue;
        }

        /* 2. Comments or division operators */
        if (c == '/') {
            if (source[i + 1] == '/') {
                /* Single-line comment */
                int start = i;
                i += 2;
                while (source[i] != '\0' && source[i] != '\n') {
                    i++;
                }
                int len = i - start;
                if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
                strncpy(tokens[token_count].lexeme, &source[start], len);
                tokens[token_count].lexeme[len] = '\0';
                tokens[token_count].type = TOKEN_COMMENT;
                tokens[token_count].token_number = token_count + 1;
                tokens[token_count].line_number = line_number;
                token_count++;
                continue;
            } else if (source[i + 1] == '*') {
                /* Multi-line comment */
                int start = i;
                i += 2;
                while (source[i] != '\0') {
                    if (source[i] == '\n') line_number++;
                    if (source[i] == '*' && source[i + 1] == '/') {
                        i += 2;
                        break;
                    }
                    i++;
                }
                int len = i - start;
                if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
                strncpy(tokens[token_count].lexeme, &source[start], len);
                tokens[token_count].lexeme[len] = '\0';
                tokens[token_count].type = TOKEN_COMMENT;
                tokens[token_count].token_number = token_count + 1;
                tokens[token_count].line_number = line_number;
                token_count++;
                continue;
            }
        }

        /* 3. String literals */
        if (c == '"') {
            int start = i;
            i++; /* skip opening quote */
            while (source[i] != '\0' && source[i] != '"') {
                if (source[i] == '\\' && source[i + 1] != '\0') {
                    i += 2; /* skip escaped character */
                } else {
                    if (source[i] == '\n') line_number++;
                    i++;
                }
            }
            if (source[i] == '"') {
                i++; /* skip closing quote */
            }
            int len = i - start;
            if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
            strncpy(tokens[token_count].lexeme, &source[start], len);
            tokens[token_count].lexeme[len] = '\0';
            tokens[token_count].type = TOKEN_STRING_LITERAL;
            tokens[token_count].token_number = token_count + 1;
            tokens[token_count].line_number = line_number;
            token_count++;
            continue;
        }

        /* 4. Character constants */
        if (c == '\'') {
            int start = i;
            i++; /* skip opening single quote */
            while (source[i] != '\0' && source[i] != '\'') {
                if (source[i] == '\\' && source[i + 1] != '\0') {
                    i += 2; /* skip escaped character */
                } else {
                    if (source[i] == '\n') line_number++;
                    i++;
                }
            }
            if (source[i] == '\'') {
                i++; /* skip closing single quote */
            }
            int len = i - start;
            if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
            strncpy(tokens[token_count].lexeme, &source[start], len);
            tokens[token_count].lexeme[len] = '\0';
            tokens[token_count].type = TOKEN_CHARACTER_CONSTANT;
            tokens[token_count].token_number = token_count + 1;
            tokens[token_count].line_number = line_number;
            token_count++;
            continue;
        }

        /* 5. Identifiers and Keywords */
        if (isalpha((unsigned char)c) || c == '_') {
            int start = i;
            while (source[i] != '\0' && (isalnum((unsigned char)source[i]) || source[i] == '_')) {
                i++;
            }
            int len = i - start;
            if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
            strncpy(tokens[token_count].lexeme, &source[start], len);
            tokens[token_count].lexeme[len] = '\0';

            if (is_keyword(tokens[token_count].lexeme)) {
                tokens[token_count].type = TOKEN_KEYWORD;
            } else {
                tokens[token_count].type = TOKEN_IDENTIFIER;
            }
            tokens[token_count].token_number = token_count + 1;
            tokens[token_count].line_number = line_number;
            token_count++;
            continue;
        }

        /* 6. Numbers (Integer & Float) */
        if (isdigit((unsigned char)c) || (c == '.' && isdigit((unsigned char)source[i + 1]))) {
            int start = i;
            int is_float = 0;

            if (c == '.') {
                is_float = 1;
                i++;
            }

            /* Read digits */
            while (source[i] != '\0' && isdigit((unsigned char)source[i])) {
                i++;
            }

            /* Check for decimal point if not already float */
            if (!is_float && source[i] == '.' && isdigit((unsigned char)source[i + 1])) {
                is_float = 1;
                i++; /* skip '.' */
                while (source[i] != '\0' && isdigit((unsigned char)source[i])) {
                    i++;
                }
            }

            /* Check for scientific notation (e or E) */
            if (source[i] == 'e' || source[i] == 'E') {
                int exp_start = i;
                i++;
                if (source[i] == '+' || source[i] == '-') {
                    i++;
                }
                if (isdigit((unsigned char)source[i])) {
                    is_float = 1;
                    while (source[i] != '\0' && isdigit((unsigned char)source[i])) {
                        i++;
                    }
                } else {
                    i = exp_start;
                }
            }

            /* Check for number suffixes (f, F, l, L, u, U) */
            if (is_float) {
                if (source[i] == 'f' || source[i] == 'F' || source[i] == 'l' || source[i] == 'L') {
                    i++;
                }
            } else {
                while (source[i] == 'u' || source[i] == 'U' || source[i] == 'l' || source[i] == 'L') {
                    i++;
                }
            }

            int len = i - start;
            if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
            strncpy(tokens[token_count].lexeme, &source[start], len);
            tokens[token_count].lexeme[len] = '\0';
            tokens[token_count].type = is_float ? TOKEN_FLOAT_CONSTANT : TOKEN_INTEGER_CONSTANT;
            tokens[token_count].token_number = token_count + 1;
            tokens[token_count].line_number = line_number;
            token_count++;
            continue;
        }

        /* 7. Operators (Multi-char first) */
        /* 3-char operators: <<=, >>= */
        if ((strncmp(&source[i], "<<=", 3) == 0) || (strncmp(&source[i], ">>=", 3) == 0)) {
            strncpy(tokens[token_count].lexeme, &source[i], 3);
            tokens[token_count].lexeme[3] = '\0';
            tokens[token_count].type = TOKEN_OPERATOR;
            tokens[token_count].token_number = token_count + 1;
            tokens[token_count].line_number = line_number;
            token_count++;
            i += 3;
            continue;
        }

        /* 2-char operators */
        if ((strncmp(&source[i], "+=", 2) == 0) || (strncmp(&source[i], "-=", 2) == 0) ||
            (strncmp(&source[i], "*=", 2) == 0) || (strncmp(&source[i], "/=", 2) == 0) ||
            (strncmp(&source[i], "%=", 2) == 0) || (strncmp(&source[i], "&=", 2) == 0) ||
            (strncmp(&source[i], "|=", 2) == 0) || (strncmp(&source[i], "^=", 2) == 0) ||
            (strncmp(&source[i], "<<", 2) == 0) || (strncmp(&source[i], ">>", 2) == 0) ||
            (strncmp(&source[i], "<=", 2) == 0) || (strncmp(&source[i], ">=", 2) == 0) ||
            (strncmp(&source[i], "==", 2) == 0) || (strncmp(&source[i], "!=", 2) == 0) ||
            (strncmp(&source[i], "&&", 2) == 0) || (strncmp(&source[i], "||", 2) == 0) ||
            (strncmp(&source[i], "++", 2) == 0) || (strncmp(&source[i], "--", 2) == 0) ||
            (strncmp(&source[i], "->", 2) == 0)) {
            strncpy(tokens[token_count].lexeme, &source[i], 2);
            tokens[token_count].lexeme[2] = '\0';
            tokens[token_count].type = TOKEN_OPERATOR;
            tokens[token_count].token_number = token_count + 1;
            tokens[token_count].line_number = line_number;
            token_count++;
            i += 2;
            continue;
        }

        /* 1-char operators */
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
            c == '=' || c == '<' || c == '>' || c == '!' || c == '&' ||
            c == '|' || c == '^' || c == '~' || c == '?' || c == ':' || c == '.') {
            tokens[token_count].lexeme[0] = c;
            tokens[token_count].lexeme[1] = '\0';
            tokens[token_count].type = TOKEN_OPERATOR;
            tokens[token_count].token_number = token_count + 1;
            tokens[token_count].line_number = line_number;
            token_count++;
            i++;
            continue;
        }

        /* 8. Separators: ;, , */
        if (c == ';' || c == ',') {
            tokens[token_count].lexeme[0] = c;
            tokens[token_count].lexeme[1] = '\0';
            tokens[token_count].type = TOKEN_SEPARATOR;
            tokens[token_count].token_number = token_count + 1;
            tokens[token_count].line_number = line_number;
            token_count++;
            i++;
            continue;
        }

        /* 9. Parentheses / Brackets / Braces: (, ), {, }, [, ] */
        if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']') {
            tokens[token_count].lexeme[0] = c;
            tokens[token_count].lexeme[1] = '\0';
            tokens[token_count].type = TOKEN_PARENTHESIS;
            tokens[token_count].token_number = token_count + 1;
            tokens[token_count].line_number = line_number;
            token_count++;
            i++;
            continue;
        }

        /* 10. Unknown Character */
        tokens[token_count].lexeme[0] = c;
        tokens[token_count].lexeme[1] = '\0';
        tokens[token_count].type = TOKEN_UNKNOWN;
        tokens[token_count].token_number = token_count + 1;
        tokens[token_count].line_number = line_number;
        token_count++;
        i++;
    }

    return token_count;
}

void print_tokens(const Token *tokens, int count) {
    if (!tokens || count <= 0) {
        printf("\nNo tokens found or source code was empty.\n");
        return;
    }

    printf("\n----------------------------------------------------------------------\n");
    printf("%-6s %-24s %-24s\n", "No.", "Lexeme", "Token Type");
    printf("----------------------------------------------------------------------\n");

    char formatted[MAX_LEXEME_LEN * 2];
    for (int i = 0; i < count; i++) {
        format_lexeme_display(tokens[i].lexeme, formatted, sizeof(formatted));
        printf("%-6d %-24s %-24s\n", tokens[i].token_number, formatted, token_type_to_string(tokens[i].type));
    }

    printf("----------------------------------------------------------------------\n");
    printf("Total Tokens Generated: %d\n", count);
}

int save_tokens_to_file(const Token *tokens, int count, const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        printf("\nError: Could not open file '%s' for writing.\n", filepath);
        return 0;
    }

    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "%-6s %-24s %-24s\n", "No.", "Lexeme", "Token Type");
    fprintf(fp, "----------------------------------------------------------------------\n");

    char formatted[MAX_LEXEME_LEN * 2];
    for (int i = 0; i < count; i++) {
        format_lexeme_display(tokens[i].lexeme, formatted, sizeof(formatted));
        fprintf(fp, "%-6d %-24s %-24s\n", tokens[i].token_number, formatted, token_type_to_string(tokens[i].type));
    }

    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "Total Tokens Generated: %d\n", count);

    fclose(fp);
    return 1;
}

void run_lexical_analyzer(void) {
    int choice = 0;
    static Token tokens[MAX_TOKENS];
    static char source_code[MAX_SOURCE_LEN];

    while (1) {
        printf("\n========================================\n");
        printf("          LEXICAL ANALYZER\n");
        printf("========================================\n\n");
        printf("1. Enter Source Code Manually\n");
        printf("2. Read Source Code From File\n");
        printf("3. Back to Main Menu\n\n");
        printf("Enter your choice: ");
        fflush(stdout);

        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid input! Please enter a number between 1 and 3.\n");
            clear_input_buffer();
            press_enter_to_continue();
            continue;
        }

        clear_input_buffer();

        if (choice == 3) {
            break;
        }

        source_code[0] = '\0';

        if (choice == 1) {
            printf("\nEnter C source code below.\n");
            printf("Type 'END' on a separate line to finish input:\n\n");

            char line[1024];
            size_t current_len = 0;

            while (1) {
                printf("> ");
                fflush(stdout);
                if (!fgets(line, sizeof(line), stdin)) {
                    break;
                }

                /* Check for END marker */
                if (strcmp(line, "END\n") == 0 || strcmp(line, "END\r\n") == 0 || strcmp(line, "END") == 0) {
                    break;
                }

                size_t line_len = strlen(line);
                if (current_len + line_len < MAX_SOURCE_LEN - 1) {
                    strcpy(source_code + current_len, line);
                    current_len += line_len;
                } else {
                    printf("\nWarning: Maximum source buffer size reached. Truncating input.\n");
                    break;
                }
            }

            if (current_len == 0) {
                printf("\nSource code is empty.\n");
                press_enter_to_continue();
                continue;
            }

            int token_count = tokenize_source(source_code, tokens, MAX_TOKENS);
            print_tokens(tokens, token_count);

            if (save_tokens_to_file(tokens, token_count, "output/tokens.txt")) {
                printf("\nTokens successfully saved to output/tokens.txt\n");
            }
            press_enter_to_continue();

        } else if (choice == 2) {
            char filepath[256];
            printf("\nEnter input file path [Default: input/lexical_input.txt]: ");
            fflush(stdout);

            if (!fgets(filepath, sizeof(filepath), stdin)) {
                strcpy(filepath, "input/lexical_input.txt");
            } else {
                filepath[strcspn(filepath, "\r\n")] = '\0';
                if (strlen(filepath) == 0) {
                    strcpy(filepath, "input/lexical_input.txt");
                }
            }

            FILE *fp = fopen(filepath, "r");
            if (!fp) {
                printf("\nError: Could not open file '%s'. Make sure the file exists.\n", filepath);
                press_enter_to_continue();
                continue;
            }

            size_t bytes_read = fread(source_code, 1, MAX_SOURCE_LEN - 1, fp);
            source_code[bytes_read] = '\0';
            fclose(fp);

            if (bytes_read == 0) {
                printf("\nFile '%s' is empty.\n", filepath);
                press_enter_to_continue();
                continue;
            }

            printf("\nSource code loaded from '%s':\n", filepath);
            printf("----------------------------------------\n");
            printf("%s\n", source_code);
            printf("----------------------------------------\n");

            int token_count = tokenize_source(source_code, tokens, MAX_TOKENS);
            print_tokens(tokens, token_count);

            if (save_tokens_to_file(tokens, token_count, "output/tokens.txt")) {
                printf("\nTokens successfully saved to output/tokens.txt\n");
            }
            press_enter_to_continue();

        } else {
            printf("\nInvalid choice! Please select an option between 1 and 3.\n");
            press_enter_to_continue();
        }
    }
}
