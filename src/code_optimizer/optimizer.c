#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "optimizer.h"
#include "../common/common.h"

/* Helper: Trim leading and trailing whitespace */
static void trim_string(char *str) {
    if (!str) return;
    
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\r' || str[len - 1] == '\n' || isspace((unsigned char)str[len - 1]))) {
        str[len - 1] = '\0';
        len--;
    }

    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

/* Helper: Check if string represents an integer constant */
static int is_numeric(const char *str) {
    if (!str || *str == '\0') return 0;
    int i = 0;
    if (str[0] == '-' || str[0] == '+') {
        if (str[1] == '\0') return 0;
        i = 1;
    }
    for (; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) return 0;
    }
    return 1;
}

/* Helper: Check if string is a valid variable identifier */
static int is_identifier(const char *str) {
    if (!str || *str == '\0') return 0;
    if (!isalpha((unsigned char)str[0]) && str[0] != '_') return 0;
    for (int i = 1; str[i] != '\0'; i++) {
        if (!isalnum((unsigned char)str[i]) && str[i] != '_') return 0;
    }
    return 1;
}

/* Constant Table Helper Functions */
static void const_table_set(ConstantTable *table, const char *var, int val) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].name, var) == 0) {
            table->entries[i].value = val;
            table->entries[i].is_constant = 1;
            return;
        }
    }
    if (table->count < MAX_CONSTANTS) {
        strncpy(table->entries[table->count].name, var, MAX_VAR_LEN - 1);
        table->entries[table->count].name[MAX_VAR_LEN - 1] = '\0';
        table->entries[table->count].value = val;
        table->entries[table->count].is_constant = 1;
        table->count++;
    }
}

static void const_table_remove(ConstantTable *table, const char *var) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].name, var) == 0) {
            table->entries[i].is_constant = 0;
            return;
        }
    }
}

static int const_table_get(const ConstantTable *table, const char *var, int *val) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].name, var) == 0 && table->entries[i].is_constant) {
            *val = table->entries[i].value;
            return 1;
        }
    }
    return 0;
}

/* Parse a single line into a Statement structure */
static int parse_statement(const char *line, Statement *stmt) {
    memset(stmt, 0, sizeof(Statement));
    strncpy(stmt->original_text, line, MAX_STMT_LEN - 1);
    stmt->original_text[MAX_STMT_LEN - 1] = '\0';

    char temp[MAX_STMT_LEN];
    strncpy(temp, line, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    trim_string(temp);

    if (strlen(temp) == 0) return 0;

    char *eq_ptr = strchr(temp, '=');
    if (!eq_ptr) {
        stmt->type = STMT_INVALID;
        snprintf(stmt->error_msg, sizeof(stmt->error_msg), "ERROR: Invalid statement format.");
        return 0;
    }

    *eq_ptr = '\0';
    char *lhs_str = temp;
    char *rhs_str = eq_ptr + 1;

    trim_string(lhs_str);
    trim_string(rhs_str);

    if (!is_identifier(lhs_str)) {
        stmt->type = STMT_INVALID;
        snprintf(stmt->error_msg, sizeof(stmt->error_msg), "ERROR: Invalid statement format.");
        return 0;
    }

    strncpy(stmt->lhs, lhs_str, MAX_VAR_LEN - 1);
    stmt->lhs[MAX_VAR_LEN - 1] = '\0';

    if (strlen(rhs_str) == 0) {
        stmt->type = STMT_INVALID;
        snprintf(stmt->error_msg, sizeof(stmt->error_msg), "ERROR: Invalid statement format.");
        return 0;
    }

    /* Look for operators +, -, *, / in rhs_str */
    char found_op = '\0';
    int op_idx = -1;

    for (int i = 0; rhs_str[i] != '\0'; i++) {
        char c = rhs_str[i];
        if (c == '+' || c == '*' || c == '/') {
            found_op = c;
            op_idx = i;
            break;
        } else if (c == '-') {
            if (i > 0 && !isspace((unsigned char)rhs_str[i - 1]) && rhs_str[i - 1] != '+' && rhs_str[i - 1] != '*' && rhs_str[i - 1] != '/') {
                found_op = c;
                op_idx = i;
                break;
            } else if (i > 0) {
                found_op = c;
                op_idx = i;
                break;
            }
        }
    }

    if (found_op != '\0' && op_idx >= 0) {
        stmt->op[0] = found_op;
        stmt->op[1] = '\0';

        char op1_buf[MAX_STMT_LEN];
        char op2_buf[MAX_STMT_LEN];

        strncpy(op1_buf, rhs_str, op_idx);
        op1_buf[op_idx] = '\0';
        strncpy(op2_buf, rhs_str + op_idx + 1, sizeof(op2_buf) - 1);
        op2_buf[sizeof(op2_buf) - 1] = '\0';

        trim_string(op1_buf);
        trim_string(op2_buf);

        if ((!is_numeric(op1_buf) && !is_identifier(op1_buf)) ||
            (!is_numeric(op2_buf) && !is_identifier(op2_buf))) {
            stmt->type = STMT_INVALID;
            snprintf(stmt->error_msg, sizeof(stmt->error_msg), "ERROR: Invalid statement format.");
            return 0;
        }

        strncpy(stmt->op1, op1_buf, MAX_VAR_LEN - 1);
        stmt->op1[MAX_VAR_LEN - 1] = '\0';
        strncpy(stmt->op2, op2_buf, MAX_VAR_LEN - 1);
        stmt->op2[MAX_VAR_LEN - 1] = '\0';
        stmt->type = STMT_BINARY_OP;
    } else {
        if (is_numeric(rhs_str)) {
            stmt->type = STMT_ASSIGN_CONST;
            strncpy(stmt->op1, rhs_str, MAX_VAR_LEN - 1);
            stmt->op1[MAX_VAR_LEN - 1] = '\0';
            stmt->op[0] = '\0';
            stmt->op2[0] = '\0';
        } else if (is_identifier(rhs_str)) {
            stmt->type = STMT_ASSIGN_VAR;
            strncpy(stmt->op1, rhs_str, MAX_VAR_LEN - 1);
            stmt->op1[MAX_VAR_LEN - 1] = '\0';
            stmt->op[0] = '\0';
            stmt->op2[0] = '\0';
        } else {
            stmt->type = STMT_INVALID;
            snprintf(stmt->error_msg, sizeof(stmt->error_msg), "ERROR: Invalid statement format.");
            return 0;
        }
    }

    return 1;
}

/* Convert statement to formatted code string */
static void statement_to_string(const Statement *s, char *buf, size_t buf_size) {
    if (s->type == STMT_ASSIGN_CONST || s->type == STMT_ASSIGN_VAR) {
        snprintf(buf, buf_size, "%s = %s", s->lhs, s->op1);
    } else if (s->type == STMT_BINARY_OP) {
        snprintf(buf, buf_size, "%s = %s %s %s", s->lhs, s->op1, s->op, s->op2);
    } else {
        snprintf(buf, buf_size, "%s", s->original_text);
    }
}

/* Perform Code Optimization Pipeline */
int optimize_code(const char *input_text, OptimizerResult *result) {
    if (!result) return 0;
    memset(result, 0, sizeof(OptimizerResult));

    if (!input_text || strlen(input_text) == 0) {
        result->has_error = 1;
        strncpy(result->error_msg, "ERROR: Input is empty.", sizeof(result->error_msg) - 1);
        result->executed = 1;
        return 0;
    }

    /* 1. Parse Input Statements */
    const char *ptr = input_text;
    char line_buf[MAX_STMT_LEN];

    while (*ptr != '\0' && result->original_count < MAX_STATEMENTS) {
        int pos = 0;
        while (*ptr != '\0' && *ptr != '\n' && pos < MAX_STMT_LEN - 1) {
            line_buf[pos++] = *ptr++;
        }
        if (*ptr == '\n') ptr++;
        line_buf[pos] = '\0';

        trim_string(line_buf);
        if (strlen(line_buf) == 0) continue;

        Statement *orig_stmt = &result->original_stmts[result->original_count];
        parse_statement(line_buf, orig_stmt);

        if (orig_stmt->type == STMT_INVALID && !result->has_error) {
            result->has_error = 1;
            strncpy(result->error_msg, orig_stmt->error_msg, sizeof(result->error_msg) - 1);
        }

        result->original_count++;
    }

    if (result->original_count == 0) {
        result->has_error = 1;
        strncpy(result->error_msg, "ERROR: Empty input.", sizeof(result->error_msg) - 1);
        result->executed = 1;
        return 0;
    }

    /* Copy original statements to optimized list */
    for (int i = 0; i < result->original_count; i++) {
        result->optimized_stmts[i] = result->original_stmts[i];
    }
    result->optimized_count = result->original_count;

    ConstantTable ctable;
    memset(&ctable, 0, sizeof(ConstantTable));

    /* Optimization Passes in order:
       1. Algebraic Simplification (first check identity elements)
       2. Constant Propagation
       3. Constant Folding */
    for (int i = 0; i < result->optimized_count; i++) {
        Statement *s = &result->optimized_stmts[i];
        if (s->type == STMT_INVALID) continue;

        /* Step A: Algebraic Simplification */
        if (s->type == STMT_BINARY_OP) {
            if (s->op[0] == '+') {
                if (strcmp(s->op2, "0") == 0) {
                    s->type = is_numeric(s->op1) ? STMT_ASSIGN_CONST : STMT_ASSIGN_VAR;
                    s->op[0] = '\0';
                    s->op2[0] = '\0';
                } else if (strcmp(s->op1, "0") == 0) {
                    strncpy(s->op1, s->op2, MAX_VAR_LEN - 1);
                    s->op1[MAX_VAR_LEN - 1] = '\0';
                    s->type = is_numeric(s->op1) ? STMT_ASSIGN_CONST : STMT_ASSIGN_VAR;
                    s->op[0] = '\0';
                    s->op2[0] = '\0';
                }
            } else if (s->op[0] == '-') {
                if (strcmp(s->op2, "0") == 0) {
                    s->type = is_numeric(s->op1) ? STMT_ASSIGN_CONST : STMT_ASSIGN_VAR;
                    s->op[0] = '\0';
                    s->op2[0] = '\0';
                }
            } else if (s->op[0] == '*') {
                if (strcmp(s->op2, "1") == 0) {
                    s->type = is_numeric(s->op1) ? STMT_ASSIGN_CONST : STMT_ASSIGN_VAR;
                    s->op[0] = '\0';
                    s->op2[0] = '\0';
                } else if (strcmp(s->op1, "1") == 0) {
                    strncpy(s->op1, s->op2, MAX_VAR_LEN - 1);
                    s->op1[MAX_VAR_LEN - 1] = '\0';
                    s->type = is_numeric(s->op1) ? STMT_ASSIGN_CONST : STMT_ASSIGN_VAR;
                    s->op[0] = '\0';
                    s->op2[0] = '\0';
                } else if (strcmp(s->op2, "0") == 0 || strcmp(s->op1, "0") == 0) {
                    s->type = STMT_ASSIGN_CONST;
                    strncpy(s->op1, "0", MAX_VAR_LEN - 1);
                    s->op[0] = '\0';
                    s->op2[0] = '\0';
                }
            } else if (s->op[0] == '/') {
                if (strcmp(s->op2, "1") == 0) {
                    s->type = is_numeric(s->op1) ? STMT_ASSIGN_CONST : STMT_ASSIGN_VAR;
                    s->op[0] = '\0';
                    s->op2[0] = '\0';
                } else if (strcmp(s->op1, "0") == 0 && strcmp(s->op2, "0") != 0) {
                    s->type = STMT_ASSIGN_CONST;
                    strncpy(s->op1, "0", MAX_VAR_LEN - 1);
                    s->op[0] = '\0';
                    s->op2[0] = '\0';
                }
            }
        }

        /* Step B: Constant Propagation */
        if (s->type == STMT_BINARY_OP) {
            int val1 = 0, val2 = 0;
            if (!is_numeric(s->op1) && const_table_get(&ctable, s->op1, &val1)) {
                snprintf(s->op1, MAX_VAR_LEN, "%d", val1);
            }
            if (!is_numeric(s->op2) && const_table_get(&ctable, s->op2, &val2)) {
                snprintf(s->op2, MAX_VAR_LEN, "%d", val2);
            }
        } else if (s->type == STMT_ASSIGN_VAR) {
            int val1 = 0;
            /* Only propagate constant to STMT_ASSIGN_VAR if op1 was directly defined as a constant integer (e.g. a = 10; b = a) */
            if (!is_numeric(s->op1) && const_table_get(&ctable, s->op1, &val1)) {
                /* Check if s->op1 was defined as direct constant in original statement */
                for (int orig = 0; orig < i; orig++) {
                    if (strcmp(result->original_stmts[orig].lhs, s->op1) == 0 &&
                        result->original_stmts[orig].type == STMT_ASSIGN_CONST) {
                        snprintf(s->op1, MAX_VAR_LEN, "%d", val1);
                        s->type = STMT_ASSIGN_CONST;
                        break;
                    }
                }
            }
        }

        /* Step C: Constant Folding */
        if (s->type == STMT_BINARY_OP && is_numeric(s->op1) && is_numeric(s->op2)) {
            int num1 = atoi(s->op1);
            int num2 = atoi(s->op2);
            int res = 0;
            int fold_ok = 1;

            if (s->op[0] == '+') res = num1 + num2;
            else if (s->op[0] == '-') res = num1 - num2;
            else if (s->op[0] == '*') res = num1 * num2;
            else if (s->op[0] == '/') {
                if (num2 == 0) {
                    fold_ok = 0;
                    result->has_error = 1;
                    snprintf(s->error_msg, sizeof(s->error_msg), "ERROR: Division by zero. Statement not optimized.");
                    snprintf(result->error_msg, sizeof(result->error_msg), "ERROR: Division by zero. Statement not optimized.");
                } else {
                    res = num1 / num2;
                }
            }

            if (fold_ok) {
                s->type = STMT_ASSIGN_CONST;
                snprintf(s->op1, MAX_VAR_LEN, "%d", res);
                s->op[0] = '\0';
                s->op2[0] = '\0';
            }
        }

        /* Update Constant Table */
        if (s->type == STMT_ASSIGN_CONST && is_numeric(s->op1)) {
            const_table_set(&ctable, s->lhs, atoi(s->op1));
        } else {
            const_table_remove(&ctable, s->lhs);
        }
    }

    /* 5. Simple Dead Code Elimination */
    int any_var_referenced_later = 0;
    for (int i = 0; i < result->original_count; i++) {
        const Statement *s = &result->original_stmts[i];
        for (int j = i + 1; j < result->original_count; j++) {
            const Statement *next_s = &result->original_stmts[j];
            if (strcmp(next_s->op1, s->lhs) == 0 ||
                (next_s->type == STMT_BINARY_OP && strcmp(next_s->op2, s->lhs) == 0)) {
                any_var_referenced_later = 1;
                break;
            }
        }
        if (any_var_referenced_later) break;
    }

    if (any_var_referenced_later) {
        for (int i = 0; i < result->optimized_count; i++) {
            Statement *s = &result->optimized_stmts[i];
            if (s->type == STMT_INVALID) continue;

            if (i == result->optimized_count - 1) {
                s->is_dead = 0;
                continue;
            }

            int is_used = 0;
            for (int j = i + 1; j < result->optimized_count; j++) {
                const Statement *next_s = &result->original_stmts[j];
                if (strcmp(next_s->op1, s->lhs) == 0 ||
                    (next_s->type == STMT_BINARY_OP && strcmp(next_s->op2, s->lhs) == 0)) {
                    is_used = 1;
                    break;
                }
            }

            if (!is_used) {
                s->is_dead = 1;
            } else {
                s->is_dead = 0;
            }
        }
    }

    result->executed = 1;
    return !result->has_error;
}

/* Display Optimization Results to Console */
void display_optimizer_result(const OptimizerResult *result) {
    if (!result || !result->executed) {
        printf("\nNo code optimized yet. Run option 1 or 2 first.\n");
        return;
    }

    printf("\n========================================\n");
    printf("CODE OPTIMIZER\n");
    printf("========================================\n\n");

    printf("ORIGINAL CODE\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < result->original_count; i++) {
        char buf[MAX_STMT_LEN];
        statement_to_string(&result->original_stmts[i], buf, sizeof(buf));
        printf("%s\n", buf);
    }

    printf("\nOPTIMIZATION APPLIED\n");
    printf("----------------------------------------\n");
    printf("1. Constant Folding\n");
    printf("2. Constant Propagation\n");
    printf("3. Algebraic Simplification\n");
    printf("4. Simple Dead Code Elimination\n\n");

    if (result->has_error) {
        printf("%s\n\n", result->error_msg);
    }

    printf("OPTIMIZED CODE\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < result->optimized_count; i++) {
        const Statement *s = &result->optimized_stmts[i];
        if (s->is_dead) continue;

        char buf[MAX_STMT_LEN];
        statement_to_string(s, buf, sizeof(buf));
        printf("%s\n", buf);
    }
    printf("========================================\n");
}

/* Save Optimization Results to File */
int save_optimizer_output(const OptimizerResult *result, const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        printf("\nError: Could not open file '%s' for writing.\n", filepath);
        return 0;
    }

    fprintf(fp, "========================================\n");
    fprintf(fp, "CODE OPTIMIZER\n");
    fprintf(fp, "========================================\n\n");

    fprintf(fp, "ORIGINAL CODE\n");
    fprintf(fp, "----------------------------------------\n");
    for (int i = 0; i < result->original_count; i++) {
        char buf[MAX_STMT_LEN];
        statement_to_string(&result->original_stmts[i], buf, sizeof(buf));
        fprintf(fp, "%s\n", buf);
    }

    fprintf(fp, "\nOPTIMIZATION APPLIED\n");
    fprintf(fp, "----------------------------------------\n");
    fprintf(fp, "1. Constant Folding\n");
    fprintf(fp, "2. Constant Propagation\n");
    fprintf(fp, "3. Algebraic Simplification\n");
    fprintf(fp, "4. Simple Dead Code Elimination\n\n");

    if (result->has_error) {
        fprintf(fp, "%s\n\n", result->error_msg);
    }

    fprintf(fp, "OPTIMIZED CODE\n");
    fprintf(fp, "----------------------------------------\n");
    for (int i = 0; i < result->optimized_count; i++) {
        const Statement *s = &result->optimized_stmts[i];
        if (s->is_dead) continue;

        char buf[MAX_STMT_LEN];
        statement_to_string(s, buf, sizeof(buf));
        fprintf(fp, "%s\n", buf);
    }
    fprintf(fp, "========================================\n");

    fclose(fp);
    return 1;
}

/* Main Interactive Menu for Code Optimizer */
void run_code_optimizer(void) {
    int choice = 0;
    static OptimizerResult last_result;

    while (1) {
        printf("\n========================================\n");
        printf("           CODE OPTIMIZER\n");
        printf("========================================\n");
        printf("1. Optimize Input File\n");
        printf("2. Enter Code Directly\n");
        printf("3. Display Original Code\n");
        printf("4. Display Optimized Code\n");
        printf("5. Back to Main Menu\n");
        printf("========================================\n\n");
        printf("Enter your choice: ");
        fflush(stdout);

        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid input! Please enter a number between 1 and 5.\n");
            clear_input_buffer();
            press_enter_to_continue();
            continue;
        }

        clear_input_buffer();

        if (choice == 5) {
            break;
        }

        switch (choice) {
            case 1: {
                char filepath[256] = "input/optimizer_input.txt";
                FILE *fp = fopen(filepath, "r");
                if (!fp) {
                    printf("\nError: Input file '%s' not found.\n", filepath);
                    press_enter_to_continue();
                    break;
                }

                fseek(fp, 0, SEEK_END);
                long file_size = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                if (file_size <= 0) {
                    printf("\nError: Input file '%s' is empty.\n", filepath);
                    fclose(fp);
                    press_enter_to_continue();
                    break;
                }

                char *file_buf = (char *)malloc(file_size + 1);
                if (!file_buf) {
                    printf("\nError: Memory allocation failed.\n");
                    fclose(fp);
                    press_enter_to_continue();
                    break;
                }

                size_t read_bytes = fread(file_buf, 1, file_size, fp);
                file_buf[read_bytes] = '\0';
                fclose(fp);

                optimize_code(file_buf, &last_result);
                display_optimizer_result(&last_result);

                if (save_optimizer_output(&last_result, "output/optimizer_output.txt")) {
                    printf("\nResults saved to output/optimizer_output.txt\n");
                }

                free(file_buf);
                press_enter_to_continue();
                break;
            }

            case 2: {
                printf("\nEnter statements one per line.\n");
                printf("Type END on a separate line to finish.\n\n");
                fflush(stdout);

                char code_buf[4096] = "";
                char line[MAX_STMT_LEN];

                while (1) {
                    printf("> ");
                    fflush(stdout);
                    if (!fgets(line, sizeof(line), stdin)) break;

                    char check[MAX_STMT_LEN];
                    strncpy(check, line, sizeof(check) - 1);
                    check[sizeof(check) - 1] = '\0';
                    trim_string(check);

                    if (strcmp(check, "END") == 0 || strcmp(check, "end") == 0) {
                        break;
                    }

                    if (strlen(code_buf) + strlen(line) < sizeof(code_buf) - 1) {
                        strcat(code_buf, line);
                    }
                }

                printf("\nOPTIMIZATION COMPLETED\n");
                optimize_code(code_buf, &last_result);
                display_optimizer_result(&last_result);

                if (save_optimizer_output(&last_result, "output/optimizer_output.txt")) {
                    printf("\nResults saved to output/optimizer_output.txt\n");
                }

                press_enter_to_continue();
                break;
            }

            case 3:
                if (!last_result.executed) {
                    printf("\nNo code loaded yet. Run option 1 or 2 first.\n");
                } else {
                    printf("\n--- ORIGINAL CODE ---\n");
                    for (int i = 0; i < last_result.original_count; i++) {
                        char buf[MAX_STMT_LEN];
                        statement_to_string(&last_result.original_stmts[i], buf, sizeof(buf));
                        printf("%s\n", buf);
                    }
                }
                press_enter_to_continue();
                break;

            case 4:
                if (!last_result.executed) {
                    printf("\nNo code optimized yet. Run option 1 or 2 first.\n");
                } else {
                    printf("\n--- OPTIMIZED CODE ---\n");
                    for (int i = 0; i < last_result.optimized_count; i++) {
                        const Statement *s = &last_result.optimized_stmts[i];
                        if (s->is_dead) continue;
                        char buf[MAX_STMT_LEN];
                        statement_to_string(s, buf, sizeof(buf));
                        printf("%s\n", buf);
                    }
                }
                press_enter_to_continue();
                break;

            default:
                printf("\nInvalid choice! Please select an option between 1 and 5.\n");
                press_enter_to_continue();
                break;
        }
    }
}
