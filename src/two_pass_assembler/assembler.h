#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#define MAX_ASM_LINES 200
#define MAX_SYMBOLS_ASM 100
#define MAX_LINE_LEN 256
#define MAX_LABEL_LEN 50

typedef enum {
    TYPE_IS, /* Imperative Statement */
    TYPE_AD, /* Assembler Directive */
    TYPE_DL, /* Declarative Statement */
    TYPE_INVALID
} OpcodeType;

typedef struct {
    char mnemonic[10];
    OpcodeType type;
    int code;
} OpcodeInfo;

typedef struct {
    char name[MAX_LABEL_LEN];
    int address;
    int defined;
} AsmSymbol;

typedef struct {
    AsmSymbol symbols[MAX_SYMBOLS_ASM];
    int count;
} AsmSymbolTable;

typedef struct {
    int address;                      /* Memory address (-1 for directives like START/END) */
    char statement_type[10];          /* IS, AD, DL */
    int opcode;                       /* Opcode code */
    int reg_code;                     /* Register code (0 if none) */
    char operand_type[10];            /* S (symbol), C (constant), or empty */
    int operand_val;                  /* Symbol index (1-based) or Constant value */
    char operand_name[MAX_LABEL_LEN]; /* Symbol name if symbol reference */
    char raw_line[MAX_LINE_LEN];
} IntermediateEntry;

typedef struct {
    IntermediateEntry entries[MAX_ASM_LINES];
    int count;
    int start_address;
} IntermediateCode;

typedef struct {
    int address;
    int opcode;
    int reg_code;
    int operand_address;
    char type[10];                    /* "IS", "DL", "DS", "AD" */
    char raw_line[MAX_LINE_LEN];
} ObjectCodeEntry;

typedef struct {
    ObjectCodeEntry entries[MAX_ASM_LINES];
    int count;
} ObjectCode;

/* Global state declarations for assembler */
extern AsmSymbolTable g_asm_symtab;
extern IntermediateCode g_inter_code;
extern ObjectCode g_obj_code;
extern int g_pass1_done;
extern int g_pass2_done;

/* Core Assembler API */
void run_two_pass_assembler(void);
void init_assembler_state(void);

/* Helper functions */
const OpcodeInfo *lookup_opcode(const char *token);
int get_register_code(const char *reg);
int find_asm_symbol(const AsmSymbolTable *st, const char *name);
int add_asm_symbol(AsmSymbolTable *st, const char *name, int address);

/* Pass 1 & Pass 2 Functions */
int execute_pass1(const char *filepath);
int execute_pass2(void);

/* Display & Output Functions */
void display_asm_symbol_table(void);
void display_intermediate_code(void);
void display_object_code(void);

int save_asm_symbol_table(const char *filepath);
int save_intermediate_code(const char *filepath);
int save_object_code(const char *filepath);

#endif /* ASSEMBLER_H */
