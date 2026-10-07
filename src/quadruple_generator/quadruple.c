#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quadruple.h"
#include "../common/common.h"

typedef enum {
    TOK_OPERAND,
    TOK_OPERATOR,
    TOK_LPAREN,
    TOK_RPAREN
} TokenTypeQuad;

typedef struct {
    char value[MAX_VAL_LEN];
    TokenTypeQuad type;
} QuadToken;

static int tokenize_expression(const char *expr, QuadToken *tokens, int max_tokens, char *error_msg, size_t err_size) {
    int token_count = 0;
    int i = 0;

    while (expr[i] != '\0' && token_count < max_tokens) {
        char c = expr[i];

        if (isspace((unsigned char)c)) {
            i++;
            continue;
        }

        if (c == '(') {
            tokens[token_count].value[0] = '(';
            tokens[token_count].value[1] = '\0';
            tokens[token_count].type = TOK_LPAREN;
            token_count++;
            i++;
            continue;
        }

        if (c == ')') {
            tokens[token_count].value[0] = ')';
            tokens[token_count].value[1] = '\0';
            tokens[token_count].type = TOK_RPAREN;
            token_count++;
            i++;
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '/') {
            tokens[token_count].value[0] = c;
            tokens[token_count].value[1] = '\0';
            tokens[token_count].type = TOK_OPERATOR;
            token_count++;
            i++;
            continue;
        }

        if (isalnum((unsigned char)c) || c == '_' || c == '.') {
            int start = i;
            while (expr[i] != '\0' && (isalnum((unsigned char)expr[i]) || expr[i] == '_' || expr[i] == '.')) {
                i++;
            }
            int len = i - start;
            if (len >= MAX_VAL_LEN) len = MAX_VAL_LEN - 1;
            strncpy(tokens[token_count].value, &expr[start], len);
            tokens[token_count].value[len] = '\0';
            tokens[token_count].type = TOK_OPERAND;
            token_count++;
            continue;
        }

        snprintf(error_msg, err_size, "Invalid character '%c' in expression", c);
        return -1;
    }

    return token_count;
}

static int validate_tokens(const QuadToken *tokens, int count, char *error_msg, size_t err_size) {
    if (count <= 0) {
        snprintf(error_msg, err_size, "Expression is empty");
        return 0;
    }

    int open_parens = 0;

    for (int i = 0; i < count; i++) {
        if (tokens[i].type == TOK_LPAREN) {
            open_parens++;
        } else if (tokens[i].type == TOK_RPAREN) {
            open_parens--;
            if (open_parens < 0) {
                snprintf(error_msg, err_size, "ERROR: Mismatched parentheses (extra closing parenthesis)");
                return 0;
            }
        }

        if (tokens[i].type == TOK_OPERATOR) {
            if (i == 0) {
                snprintf(error_msg, err_size, "ERROR: Expression cannot start with operator '%s'", tokens[i].value);
                return 0;
            }
            if (i == count - 1) {
                snprintf(error_msg, err_size, "ERROR: Expression cannot end with operator '%s'", tokens[i].value);
                return 0;
            }
            if (tokens[i - 1].type == TOK_OPERATOR) {
                snprintf(error_msg, err_size, "ERROR: Consecutive operators '%s%s'", tokens[i - 1].value, tokens[i].value);
                return 0;
            }
            if (tokens[i - 1].type == TOK_LPAREN) {
                snprintf(error_msg, err_size, "ERROR: Operator '%s' cannot follow '('", tokens[i].value);
                return 0;
            }
            if (tokens[i + 1].type == TOK_RPAREN) {
                snprintf(error_msg, err_size, "ERROR: Operator '%s' cannot precede ')'", tokens[i].value);
                return 0;
            }
        }
    }

    if (open_parens != 0) {
        snprintf(error_msg, err_size, "ERROR: Mismatched parentheses (missing closing parenthesis)");
        return 0;
    }

    return 1;
}

static int op_precedence(const char *op) {
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return 2;
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 1;
    return 0;
}

static int infix_to_postfix(const QuadToken *tokens, int count, QuadToken *postfix, char *error_msg, size_t err_size) {
    QuadToken op_stack[MAX_TOKENS_QUAD];
    int op_top = -1;
    int post_count = 0;

    for (int i = 0; i < count; i++) {
        QuadToken tok = tokens[i];

        if (tok.type == TOK_OPERAND) {
            postfix[post_count++] = tok;
        } else if (tok.type == TOK_LPAREN) {
            op_top++;
            op_stack[op_top] = tok;
        } else if (tok.type == TOK_RPAREN) {
            while (op_top >= 0 && op_stack[op_top].type != TOK_LPAREN) {
                postfix[post_count++] = op_stack[op_top--];
            }
            if (op_top >= 0 && op_stack[op_top].type == TOK_LPAREN) {
                op_top--; /* Pop '(' */
            } else {
                snprintf(error_msg, err_size, "ERROR: Mismatched parentheses");
                return -1;
            }
        } else if (tok.type == TOK_OPERATOR) {
            while (op_top >= 0 && op_stack[op_top].type == TOK_OPERATOR &&
                   op_precedence(op_stack[op_top].value) >= op_precedence(tok.value)) {
                postfix[post_count++] = op_stack[op_top--];
            }
            op_top++;
            op_stack[op_top] = tok;
        }
    }

    while (op_top >= 0) {
        if (op_stack[op_top].type == TOK_LPAREN || op_stack[op_top].type == TOK_RPAREN) {
            snprintf(error_msg, err_size, "ERROR: Mismatched parentheses");
            return -1;
        }
        postfix[post_count++] = op_stack[op_top--];
    }

    return post_count;
}

static int generate_quads_from_postfix(const QuadToken *postfix, int post_count, QuadrupleResult *result) {
    char operand_stack[MAX_TOKENS_QUAD][MAX_VAL_LEN];
    int stack_top = -1;
    int temp_counter = 1;

    result->quad_count = 0;

    for (int i = 0; i < post_count; i++) {
        QuadToken tok = postfix[i];

        if (tok.type == TOK_OPERAND) {
            stack_top++;
            strncpy(operand_stack[stack_top], tok.value, MAX_VAL_LEN - 1);
            operand_stack[stack_top][MAX_VAL_LEN - 1] = '\0';
        } else if (tok.type == TOK_OPERATOR) {
            if (stack_top < 1) {
                snprintf(result->error_msg, sizeof(result->error_msg), "ERROR: Insufficient operands for operator '%s'", tok.value);
                return 0;
            }

            char arg2[MAX_VAL_LEN];
            char arg1[MAX_VAL_LEN];
            strncpy(arg2, operand_stack[stack_top--], MAX_VAL_LEN - 1);
            strncpy(arg1, operand_stack[stack_top--], MAX_VAL_LEN - 1);

            char temp_var[MAX_VAL_LEN];
            snprintf(temp_var, sizeof(temp_var), "T%d", temp_counter++);

            Quadruple *q = &result->quadruples[result->quad_count];
            q->index = result->quad_count + 1;
            strncpy(q->op, tok.value, sizeof(q->op) - 1);
            strncpy(q->arg1, arg1, sizeof(q->arg1) - 1);
            strncpy(q->arg2, arg2, sizeof(q->arg2) - 1);
            strncpy(q->result, temp_var, sizeof(q->result) - 1);

            result->quad_count++;

            stack_top++;
            strncpy(operand_stack[stack_top], temp_var, MAX_VAL_LEN - 1);
            operand_stack[stack_top][MAX_VAL_LEN - 1] = '\0';
        }
    }

    if (stack_top == 0) {
        strncpy(result->final_result, operand_stack[stack_top], sizeof(result->final_result) - 1);
        result->final_result[sizeof(result->final_result) - 1] = '\0';
    } else {
        snprintf(result->error_msg, sizeof(result->error_msg), "ERROR: Invalid expression syntax");
        return 0;
    }

    return 1;
}

int generate_quadruples(const char *expr, QuadrupleResult *result) {
    if (!result) return 0;
    memset(result, 0, sizeof(QuadrupleResult));

    if (!expr) {
        result->is_valid = 0;
        strncpy(result->error_msg, "Expression is NULL", sizeof(result->error_msg) - 1);
        result->executed = 1;
        return 0;
    }

    strncpy(result->expression, expr, sizeof(result->expression) - 1);
    result->expression[sizeof(result->expression) - 1] = '\0';

    size_t len = strlen(result->expression);
    while (len > 0 && (result->expression[len - 1] == '\r' || result->expression[len - 1] == '\n' || result->expression[len - 1] == ' ')) {
        result->expression[len - 1] = '\0';
        len--;
    }

    if (strlen(result->expression) == 0) {
        result->is_valid = 0;
        strncpy(result->error_msg, "Input expression is empty", sizeof(result->error_msg) - 1);
        result->executed = 1;
        return 0;
    }

    QuadToken tokens[MAX_TOKENS_QUAD];
    int token_count = tokenize_expression(result->expression, tokens, MAX_TOKENS_QUAD, result->error_msg, sizeof(result->error_msg));
    if (token_count < 0) {
        result->is_valid = 0;
        result->executed = 1;
        return 0;
    }

    if (!validate_tokens(tokens, token_count, result->error_msg, sizeof(result->error_msg))) {
        result->is_valid = 0;
        result->executed = 1;
        return 0;
    }

    QuadToken postfix[MAX_TOKENS_QUAD];
    int post_count = infix_to_postfix(tokens, token_count, postfix, result->error_msg, sizeof(result->error_msg));
    if (post_count < 0) {
        result->is_valid = 0;
        result->executed = 1;
        return 0;
    }

    if (!generate_quads_from_postfix(postfix, post_count, result)) {
        result->is_valid = 0;
        result->executed = 1;
        return 0;
    }

    result->is_valid = 1;
    result->executed = 1;
    return 1;
}

void display_quadruple_result(const QuadrupleResult *result) {
    if (!result || !result->executed) {
        printf("\nNo quadruples generated yet. Run option 1 or option 2 first.\n");
        return;
    }

    printf("\n========================================\n");
    printf("        QUADRUPLE GENERATOR\n");
    printf("========================================\n\n");
    printf("Input Expression:\n%s\n\n", result->expression);

    if (!result->is_valid) {
        printf("Result:\nINVALID EXPRESSION\n\n");
        printf("%s\n", result->error_msg);
        printf("========================================\n");
        return;
    }

    printf("Quadruple Table\n");
    printf("--------------------------------------------------\n");
    printf("%-6s %-11s %-10s %-10s %-10s\n", "No.", "Operator", "Arg1", "Arg2", "Result");
    printf("--------------------------------------------------\n");

    for (int i = 0; i < result->quad_count; i++) {
        const Quadruple *q = &result->quadruples[i];
        printf("%-6d %-11s %-10s %-10s %-10s\n", q->index, q->op, q->arg1, q->arg2, q->result);
    }
    printf("--------------------------------------------------\n\n");
    printf("Final Result:\n%s\n", result->final_result);
    printf("========================================\n");
}

int save_quadruple_output(const QuadrupleResult *result, const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        printf("\nError: Could not open file '%s' for writing.\n", filepath);
        return 0;
    }

    fprintf(fp, "========================================\n");
    fprintf(fp, "        QUADRUPLE GENERATOR\n");
    fprintf(fp, "========================================\n\n");
    fprintf(fp, "Input Expression:\n%s\n\n", result->expression);

    if (!result->is_valid) {
        fprintf(fp, "Result:\nINVALID EXPRESSION\n\n");
        fprintf(fp, "%s\n", result->error_msg);
        fprintf(fp, "========================================\n");
        fclose(fp);
        return 1;
    }

    fprintf(fp, "Quadruple Table\n");
    fprintf(fp, "--------------------------------------------------\n");
    fprintf(fp, "%-6s %-11s %-10s %-10s %-10s\n", "No.", "Operator", "Arg1", "Arg2", "Result");
    fprintf(fp, "--------------------------------------------------\n");

    for (int i = 0; i < result->quad_count; i++) {
        const Quadruple *q = &result->quadruples[i];
        fprintf(fp, "%-6d %-11s %-10s %-10s %-10s\n", q->index, q->op, q->arg1, q->arg2, q->result);
    }
    fprintf(fp, "--------------------------------------------------\n\n");
    fprintf(fp, "Final Result:\n%s\n", result->final_result);
    fprintf(fp, "========================================\n");

    fclose(fp);
    return 1;
}

void run_quadruple_generator(void) {
    int choice = 0;
    static QuadrupleResult last_result = { "", {}, 0, "", 0, "", 0 };

    while (1) {
        printf("\n========================================\n");
        printf("        QUADRUPLE GENERATOR\n");
        printf("========================================\n\n");
        printf("1. Generate from Input File\n");
        printf("2. Enter Expression Directly\n");
        printf("3. Display Last Quadruples\n");
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
                char filepath[256] = "input/quadruple_input.txt";
                FILE *fp = fopen(filepath, "r");
                if (!fp) {
                    printf("\nError: Input file '%s' not found.\n", filepath);
                    press_enter_to_continue();
                    break;
                }

                char expr_buf[MAX_QUAD_EXPR_LEN];
                if (!fgets(expr_buf, sizeof(expr_buf), fp)) {
                    printf("\nError: Input file '%s' is empty.\n", filepath);
                    fclose(fp);
                    press_enter_to_continue();
                    break;
                }
                fclose(fp);

                generate_quadruples(expr_buf, &last_result);
                display_quadruple_result(&last_result);

                if (save_quadruple_output(&last_result, "output/quadruple_output.txt")) {
                    printf("\nOutput saved to output/quadruple_output.txt\n");
                }
                press_enter_to_continue();
                break;
            }

            case 2: {
                char expr_buf[MAX_QUAD_EXPR_LEN];
                printf("\nEnter expression: ");
                fflush(stdout);

                if (!fgets(expr_buf, sizeof(expr_buf), stdin)) {
                    printf("\nError reading input expression.\n");
                    press_enter_to_continue();
                    break;
                }

                generate_quadruples(expr_buf, &last_result);
                display_quadruple_result(&last_result);

                if (save_quadruple_output(&last_result, "output/quadruple_output.txt")) {
                    printf("\nOutput saved to output/quadruple_output.txt\n");
                }
                press_enter_to_continue();
                break;
            }

            case 3:
                display_quadruple_result(&last_result);
                press_enter_to_continue();
                break;

            default:
                printf("\nInvalid choice! Please select an option between 1 and 4.\n");
                press_enter_to_continue();
                break;
        }
    }
}
