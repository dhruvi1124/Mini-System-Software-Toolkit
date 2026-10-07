#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#define MAX_TOKENS 1000
#define MAX_LEXEME_LEN 256
#define MAX_SOURCE_LEN 10000

typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER_CONSTANT,
    TOKEN_FLOAT_CONSTANT,
    TOKEN_CHARACTER_CONSTANT,
    TOKEN_STRING_LITERAL,
    TOKEN_OPERATOR,
    TOKEN_SEPARATOR,
    TOKEN_PARENTHESIS,
    TOKEN_COMMENT,
    TOKEN_PREPROCESSOR_DIRECTIVE,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    int token_number;
    char lexeme[MAX_LEXEME_LEN];
    TokenType type;
    int line_number;
} Token;

/* Main entry point called from main menu */
void run_lexical_analyzer(void);

/* Lexer core modular functions */
int tokenize_source(const char *source, Token *tokens, int max_tokens);
int is_keyword(const char *word);
const char *token_type_to_string(TokenType type);
void print_tokens(const Token *tokens, int count);
int save_tokens_to_file(const Token *tokens, int count, const char *filepath);

#endif /* LEXICAL_ANALYZER_H */
