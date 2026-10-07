#ifndef PARSER_H
#define PARSER_H

#define MAX_EXPR_LEN 256

typedef struct {
    char expression[MAX_EXPR_LEN];
    int is_valid;
    int error_pos;
    char error_msg[MAX_EXPR_LEN];
    int executed;
} ParserResult;

/* Main entry point called from main menu */
void run_recursive_descent_parser(void);

/* Parser API */
int parse_expression_string(const char *expr, ParserResult *result);
void display_parser_result(const ParserResult *result);
int save_parser_output(const ParserResult *result, const char *filepath);

#endif /* PARSER_H */
