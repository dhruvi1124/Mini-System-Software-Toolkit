#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "assembler.h"
#include "../common/common.h"

static void trim_line(char *line) {
    /* Strip comments starting with ;, //, # */
    char *comment = strpbrk(line, ";#");
    if (comment) *comment = '\0';
    char *double_slash = strstr(line, "//");
    if (double_slash) *double_slash = '\0';

    /* Strip trailing carriage returns/newlines */
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || isspace((unsigned char)line[len - 1]))) {
        line[len - 1] = '\0';
        len--;
    }
}

int execute_pass1(const char *filepath) {
    init_assembler_state();

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("\nError: Assembly source file '%s' not found.\n", filepath);
        return 0;
    }

    char raw_line[MAX_LINE_LEN];
    char working[MAX_LINE_LEN];
    int lc = 0;
    int line_num = 0;
    int has_start = 0;
    int has_end = 0;

    while (fgets(raw_line, sizeof(raw_line), fp)) {
        line_num++;
        strncpy(working, raw_line, sizeof(working) - 1);
        working[sizeof(working) - 1] = '\0';

        trim_line(working);

        /* Skip empty lines */
        char *ptr = working;
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (*ptr == '\0') continue;

        /* Save raw line for IC display without trailing newline */
        char clean_raw[MAX_LINE_LEN];
        strncpy(clean_raw, ptr, sizeof(clean_raw) - 1);
        clean_raw[sizeof(clean_raw) - 1] = '\0';

        /* Parse tokens */
        char tokens[4][MAX_LABEL_LEN] = {{0}};
        int token_count = 0;

        char *token = strtok(ptr, " \t");
        while (token && token_count < 4) {
            strncpy(tokens[token_count], token, MAX_LABEL_LEN - 1);
            tokens[token_count][MAX_LABEL_LEN - 1] = '\0';
            token_count++;
            token = strtok(NULL, " \t");
        }

        if (token_count == 0) continue;

        /* Determine if first token is opcode or label */
        const OpcodeInfo *op_info = lookup_opcode(tokens[0]);
        char label[MAX_LABEL_LEN] = "";
        char mnemonic[MAX_LABEL_LEN] = "";
        char op1[MAX_LABEL_LEN] = "";
        char op2[MAX_LABEL_LEN] = "";

        if (op_info != NULL) {
            /* No label */
            strncpy(mnemonic, tokens[0], sizeof(mnemonic) - 1);
            if (token_count > 1) strncpy(op1, tokens[1], sizeof(op1) - 1);
            if (token_count > 2) strncpy(op2, tokens[2], sizeof(op2) - 1);
        } else {
            /* First token is label */
            strncpy(label, tokens[0], sizeof(label) - 1);
            if (token_count > 1) {
                strncpy(mnemonic, tokens[1], sizeof(mnemonic) - 1);
                op_info = lookup_opcode(mnemonic);
            }
            if (token_count > 2) strncpy(op1, tokens[2], sizeof(op1) - 1);
            if (token_count > 3) strncpy(op2, tokens[3], sizeof(op2) - 1);
        }

        if (op_info == NULL) {
            printf("\nError at line %d: Unknown opcode/instruction '%s'\n", line_num, mnemonic);
            fclose(fp);
            return 0;
        }

        /* If label exists, record in Symbol Table */
        if (strlen(label) > 0) {
            if (add_asm_symbol(&g_asm_symtab, label, lc) == -1) {
                fclose(fp);
                return 0;
            }
        }

        /* Build Intermediate Code Entry */
        IntermediateEntry *ie = &g_inter_code.entries[g_inter_code.count];
        memset(ie, 0, sizeof(IntermediateEntry));
        strncpy(ie->raw_line, clean_raw, sizeof(ie->raw_line) - 1);
        ie->opcode = op_info->code;

        if (op_info->type == TYPE_AD) {
            /* Assembler Directive: START, END */
            strncpy(ie->statement_type, "AD", sizeof(ie->statement_type) - 1);
            ie->address = -1;

            if (strcmp(op_info->mnemonic, "START") == 0) {
                has_start = 1;
                if (strlen(op1) > 0) {
                    lc = atoi(op1);
                } else {
                    lc = 0;
                }
                g_inter_code.start_address = lc;
                strncpy(ie->operand_type, "C", sizeof(ie->operand_type) - 1);
                ie->operand_val = lc;
            } else if (strcmp(op_info->mnemonic, "END") == 0) {
                has_end = 1;
                g_inter_code.count++;
                break;
            }
        } else if (op_info->type == TYPE_IS) {
            /* Imperative Statement */
            strncpy(ie->statement_type, "IS", sizeof(ie->statement_type) - 1);
            ie->address = lc;

            int reg_code = get_register_code(op1);
            ie->reg_code = reg_code;

            /* Check second operand (Symbol or Constant) */
            char *symbol_target = (reg_code > 0) ? op2 : op1;
            if (strlen(symbol_target) > 0) {
                /* Strip trailing commas */
                size_t st_len = strlen(symbol_target);
                if (symbol_target[st_len - 1] == ',') symbol_target[st_len - 1] = '\0';

                if (isdigit((unsigned char)symbol_target[0])) {
                    strncpy(ie->operand_type, "C", sizeof(ie->operand_type) - 1);
                    ie->operand_val = atoi(symbol_target);
                } else {
                    strncpy(ie->operand_type, "S", sizeof(ie->operand_type) - 1);
                    strncpy(ie->operand_name, symbol_target, sizeof(ie->operand_name) - 1);
                    int sym_idx = add_asm_symbol(&g_asm_symtab, symbol_target, -1);
                    ie->operand_val = sym_idx + 1; /* 1-based index */
                }
            }
            lc += 1;
        } else if (op_info->type == TYPE_DL) {
            /* Declarative Statement: DC, DS */
            strncpy(ie->statement_type, "DL", sizeof(ie->statement_type) - 1);
            ie->address = lc;

            if (strcmp(op_info->mnemonic, "DC") == 0) {
                strncpy(ie->operand_type, "C", sizeof(ie->operand_type) - 1);
                ie->operand_val = atoi(op1);
                lc += 1;
            } else if (strcmp(op_info->mnemonic, "DS") == 0) {
                strncpy(ie->operand_type, "C", sizeof(ie->operand_type) - 1);
                int size = atoi(op1);
                ie->operand_val = (size > 0) ? size : 1;
                lc += ie->operand_val;
            }
        }

        g_inter_code.count++;
    }

    fclose(fp);

    if (!has_start) {
        printf("\nNote: 'START' directive missing. Defaulting start address to 0.\n");
    }
    if (!has_end) {
        printf("\nNote: 'END' directive missing at end of assembly file.\n");
    }

    g_pass1_done = 1;
    return 1;
}
