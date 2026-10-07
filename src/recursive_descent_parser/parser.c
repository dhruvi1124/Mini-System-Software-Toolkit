#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "../common/common.h"

/* Global state for recursive descent parser execution */
static const char *g_input = NULL;
static int g_pos = 0;
static int g_error_pos = 0;
static char g_error_msg[MAX_EXPR_LEN];
static int g_has_error = 0;
static ParserResult g_last_result = { "", 0, 0, "", 0 };

static void set_error(const char *msg) {
    if (!g_has_error) {
        g_has_error = 1;
        g_error_pos = g_pos;
        strncpy(g_error_msg, msg, sizeof(g_error_msg) - 1);
        g_error_msg[sizeof(g_error_msg) - 1] = '\0';
    }
}

static void skip_whitespace(void) {
    while (g_input[g_pos] == ' ' || g_input[g_pos] == '\t' ||
           g_input[g_pos] == '\r' || g_input[g_pos] == '\n') {
        g_pos++;
    }
}

/* Grammar:
   E  -> T E'
   E' -> + T E' | - T E' | epsilon
   T  -> F T'
   T' -> * F T' | / F T' | epsilon
   F  -> ( E ) | id | number
*/

static int parse_expression(void);
static int parse_expression_prime(void);
static int parse_term(void);
static int parse_term_prime(void);
static int parse_factor(void);

/* F -> ( E ) | id | number */
static int parse_factor(void) {
    skip_whitespace();
    char c = g_input[g_pos];

    if (c == '(') {
        g_pos++; /* consume '(' */
        if (!parse_expression()) return 0;
        skip_whitespace();
        if (g_input[g_pos] == ')') {
            g_pos++; /* consume ')' */
            return 1;
        } else {
            set_error("Expected ')'");
            return 0;
        }
    } else if (isalpha((unsigned char)c) || c == '_') {
        /* Identifier: id */
        while (g_input[g_pos] != '\0' && (isalnum((unsigned char)g_input[g_pos]) || g_input[g_pos] == '_')) {
            g_pos++;
        }
        return 1;
    } else if (isdigit((unsigned char)c)) {
        /* Number */
        while (g_input[g_pos] != '\0' && isdigit((unsigned char)g_input[g_pos])) {
            g_pos++;
        }
        return 1;
    } else {
        if (c == '\0') {
            set_error("Unexpected end of expression");
        } else {
            set_error("Expected identifier, number, or '('");
        }
        return 0;
    }
}

/* T' -> * F T' | / F T' | epsilon */
static int parse_term_prime(void) {
    skip_whitespace();
    char c = g_input[g_pos];

    if (c == '*' || c == '/') {
        g_pos++; /* consume '*' or '/' */
        if (!parse_factor()) return 0;
        return parse_term_prime();
    }
    /* Epsilon production */
    return 1;
}

/* T -> F T' */
static int parse_term(void) {
    if (!parse_factor()) return 0;
    return parse_term_prime();
}

/* E' -> + T E' | - T E' | epsilon */
static int parse_expression_prime(void) {
    skip_whitespace();
    char c = g_input[g_pos];

    if (c == '+' || c == '-') {
        g_pos++; /* consume '+' or '-' */
        if (!parse_term()) return 0;
        return parse_expression_prime();
    }
    /* Epsilon production */
    return 1;
}

/* E -> T E' */
static int parse_expression(void) {
    if (!parse_term()) return 0;
    return parse_expression_prime();
}

int parse_expression_string(const char *expr, ParserResult *result) {
    if (!result) return 0;

    memset(result, 0, sizeof(ParserResult));
    if (!expr) {
        strncpy(result->error_msg, "Expression is NULL", sizeof(result->error_msg) - 1);
        result->is_valid = 0;
        result->executed = 1;
        return 0;
    }

    strncpy(result->expression, expr, sizeof(result->expression) - 1);
    result->expression[sizeof(result->expression) - 1] = '\0';

    /* Clean trailing whitespace or newlines */
    size_t len = strlen(result->expression);
    while (len > 0 && (result->expression[len - 1] == '\r' || result->expression[len - 1] == '\n' || result->expression[len - 1] == ' ')) {
        result->expression[len - 1] = '\0';
        len--;
    }

    g_input = result->expression;
    g_pos = 0;
    g_error_pos = 0;
    g_has_error = 0;
    g_error_msg[0] = '\0';

    skip_whitespace();
    if (g_input[g_pos] == '\0') {
        result->is_valid = 0;
        result->error_pos = 0;
        strncpy(result->error_msg, "Input expression is empty", sizeof(result->error_msg) - 1);
        result->executed = 1;
        g_last_result = *result;
        return 0;
    }

    if (!parse_expression()) {
        result->is_valid = 0;
        result->error_pos = g_error_pos;
        strncpy(result->error_msg, g_error_msg, sizeof(result->error_msg) - 1);
        result->executed = 1;
        g_last_result = *result;
        return 0;
    }

    skip_whitespace();
    if (g_input[g_pos] != '\0') {
        result->is_valid = 0;
        result->error_pos = g_pos;
        snprintf(result->error_msg, sizeof(result->error_msg), "Unexpected trailing character '%c'", g_input[g_pos]);
        result->executed = 1;
        g_last_result = *result;
        return 0;
    }

    result->is_valid = 1;
    result->executed = 1;
    g_last_result = *result;
    return 1;
}

void display_parser_result(const ParserResult *result) {
    if (!result || !result->executed) {
        printf("\nNo parsing result available. Run option 1 or option 2 first.\n");
        return;
    }

    printf("\n========================================\n");
    printf("        RECURSIVE DESCENT PARSER\n");
    printf("========================================\n\n");
    printf("Input Expression:\n%s\n\n", result->expression);
    printf("Grammar:\n");
    printf("E  -> T E'\n");
    printf("E' -> + T E' | - T E' | epsilon\n");
    printf("T  -> F T'\n");
    printf("T' -> * F T' | / F T' | epsilon\n");
    printf("F  -> ( E ) | id | number\n\n");
    printf("Result:\n");

    if (result->is_valid) {
        printf("VALID EXPRESSION\n\n");
        printf("Syntax analysis successful.\n");
    } else {
        printf("INVALID EXPRESSION\n\n");
        printf("Syntax error near position: %d", result->error_pos);
        if (strlen(result->error_msg) > 0) {
            printf(" (%s)", result->error_msg);
        }
        printf("\n");
    }
    printf("========================================\n");
}

int save_parser_output(const ParserResult *result, const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        printf("\nError: Could not open output file '%s'\n", filepath);
        return 0;
    }

    fprintf(fp, "========================================\n");
    fprintf(fp, "        RECURSIVE DESCENT PARSER\n");
    fprintf(fp, "========================================\n\n");
    fprintf(fp, "Input Expression:\n%s\n\n", result->expression);
    fprintf(fp, "Grammar:\n");
    fprintf(fp, "E  -> T E'\n");
    fprintf(fp, "E' -> + T E' | - T E' | epsilon\n");
    fprintf(fp, "T  -> F T'\n");
    fprintf(fp, "T' -> * F T' | / F T' | epsilon\n");
    fprintf(fp, "F  -> ( E ) | id | number\n\n");
    fprintf(fp, "Result:\n");

    if (result->is_valid) {
        fprintf(fp, "VALID EXPRESSION\n\n");
        fprintf(fp, "Syntax analysis successful.\n");
    } else {
        fprintf(fp, "INVALID EXPRESSION\n\n");
        fprintf(fp, "Syntax error near position: %d", result->error_pos);
        if (strlen(result->error_msg) > 0) {
            fprintf(fp, " (%s)", result->error_msg);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "========================================\n");

    fclose(fp);
    return 1;
}

void run_recursive_descent_parser(void) {
    int choice = 0;

    while (1) {
        printf("\n========================================\n");
        printf("      RECURSIVE DESCENT PARSER\n");
        printf("========================================\n\n");
        printf("1. Parse Input File\n");
        printf("2. Enter Expression Directly\n");
        printf("3. Display Last Result\n");
        printf("4. Back to Main Menu\n\n");
        printf("Enter your choice: ");
        fflush(stdout);

        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid input! Please enter a number between 1 and 4.\n");
            clear_input_buffer();
            press_enter_to_continue();
            continue;
        }

        clear_input_buffer();

        if (choice == 4) {
            break;
        }

        switch (choice) {
            case 1: {
                char filepath[256] = "input/parser_input.txt";
                FILE *fp = fopen(filepath, "r");
                if (!fp) {
                    printf("\nError: Input file '%s' not found.\n", filepath);
                    press_enter_to_continue();
                    break;
                }

                char expr_buf[MAX_EXPR_LEN];
                if (!fgets(expr_buf, sizeof(expr_buf), fp)) {
                    printf("\nError: Input file '%s' is empty.\n", filepath);
                    fclose(fp);
                    press_enter_to_continue();
                    break;
                }
                fclose(fp);

                ParserResult res;
                parse_expression_string(expr_buf, &res);
                display_parser_result(&res);

                if (save_parser_output(&res, "output/parser_output.txt")) {
                    printf("\nResult saved to output/parser_output.txt\n");
                }
                press_enter_to_continue();
                break;
            }

            case 2: {
                char expr_buf[MAX_EXPR_LEN];
                printf("\nEnter expression: ");
                fflush(stdout);

                if (!fgets(expr_buf, sizeof(expr_buf), stdin)) {
                    printf("\nError reading input expression.\n");
                    press_enter_to_continue();
                    break;
                }

                ParserResult res;
                parse_expression_string(expr_buf, &res);
                display_parser_result(&res);

                if (save_parser_output(&res, "output/parser_output.txt")) {
                    printf("\nResult saved to output/parser_output.txt\n");
                }
                press_enter_to_continue();
                break;
            }

            case 3:
                display_parser_result(&g_last_result);
                press_enter_to_continue();
                break;

            default:
                printf("\nInvalid choice! Please select an option between 1 and 4.\n");
                press_enter_to_continue();
                break;
        }
    }
}
