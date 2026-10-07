#ifndef QUADRUPLE_H
#define QUADRUPLE_H

#define MAX_QUAD_EXPR_LEN 256
#define MAX_QUADS 50
#define MAX_TOKENS_QUAD 100
#define MAX_VAL_LEN 50

typedef struct {
    int index;
    char op[10];
    char arg1[MAX_VAL_LEN];
    char arg2[MAX_VAL_LEN];
    char result[MAX_VAL_LEN];
} Quadruple;

typedef struct {
    char expression[MAX_QUAD_EXPR_LEN];
    Quadruple quadruples[MAX_QUADS];
    int quad_count;
    char final_result[MAX_VAL_LEN];
    int is_valid;
    char error_msg[MAX_QUAD_EXPR_LEN];
    int executed;
} QuadrupleResult;

/* Public API */
void run_quadruple_generator(void);

int generate_quadruples(const char *expr, QuadrupleResult *result);
void display_quadruple_result(const QuadrupleResult *result);
int save_quadruple_output(const QuadrupleResult *result, const char *filepath);

#endif /* QUADRUPLE_H */
