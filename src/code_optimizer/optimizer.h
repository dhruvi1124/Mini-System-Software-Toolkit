#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#define MAX_STATEMENTS 100
#define MAX_STMT_LEN 128
#define MAX_VAR_LEN 32
#define MAX_CONSTANTS 50

typedef enum {
    STMT_INVALID = 0,
    STMT_ASSIGN_CONST,     /* x = 10 */
    STMT_ASSIGN_VAR,       /* x = y */
    STMT_BINARY_OP         /* x = y + z */
} StmtType;

typedef struct {
    char lhs[MAX_VAR_LEN];
    StmtType type;
    char op1[MAX_VAR_LEN];
    char op[4];
    char op2[MAX_VAR_LEN];
    int is_dead;
    char original_text[MAX_STMT_LEN];
    char error_msg[MAX_STMT_LEN];
} Statement;

typedef struct {
    char name[MAX_VAR_LEN];
    int value;
    int is_constant;
} ConstantEntry;

typedef struct {
    ConstantEntry entries[MAX_CONSTANTS];
    int count;
} ConstantTable;

typedef struct {
    Statement original_stmts[MAX_STATEMENTS];
    int original_count;

    Statement optimized_stmts[MAX_STATEMENTS];
    int optimized_count;

    int has_error;
    char error_msg[MAX_STMT_LEN];
    int executed;
} OptimizerResult;

void run_code_optimizer(void);
int optimize_code(const char *input_text, OptimizerResult *result);
void display_optimizer_result(const OptimizerResult *result);
int save_optimizer_output(const OptimizerResult *result, const char *filepath);

#endif /* OPTIMIZER_H */
