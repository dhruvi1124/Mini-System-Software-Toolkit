#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "assembler.h"
#include "../common/common.h"

/* Global state definition */
AsmSymbolTable g_asm_symtab;
IntermediateCode g_inter_code;
ObjectCode g_obj_code;
int g_pass1_done = 0;
int g_pass2_done = 0;

static const OpcodeInfo OPCODES[] = {
    {"MOVER", TYPE_IS, 1},
    {"MOVEM", TYPE_IS, 2},
    {"ADD",   TYPE_IS, 3},
    {"SUB",   TYPE_IS, 4},
    {"MULT",  TYPE_IS, 5},
    {"DIV",   TYPE_IS, 6},
    {"COMP",  TYPE_IS, 7},
    {"BC",    TYPE_IS, 8},
    {"READ",  TYPE_IS, 9},
    {"PRINT", TYPE_IS, 10},
    {"START", TYPE_AD, 1},
    {"END",   TYPE_AD, 2},
    {"DC",    TYPE_DL, 1},
    {"DS",    TYPE_DL, 2}
};
#define NUM_OPCODES (sizeof(OPCODES) / sizeof(OPCODES[0]))

void init_assembler_state(void) {
    g_asm_symtab.count = 0;
    g_inter_code.count = 0;
    g_inter_code.start_address = 0;
    g_obj_code.count = 0;
    g_pass1_done = 0;
    g_pass2_done = 0;
}

const OpcodeInfo *lookup_opcode(const char *token) {
    if (!token || *token == '\0') return NULL;
    char temp[MAX_LABEL_LEN];
    strncpy(temp, token, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    for (int i = 0; temp[i]; i++) {
        temp[i] = (char)toupper((unsigned char)temp[i]);
    }

    for (size_t i = 0; i < NUM_OPCODES; i++) {
        if (strcmp(temp, OPCODES[i].mnemonic) == 0) {
            return &OPCODES[i];
        }
    }
    return NULL;
}

int get_register_code(const char *reg) {
    if (!reg || *reg == '\0') return 0;
    char temp[MAX_LABEL_LEN];
    strncpy(temp, reg, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    /* Strip trailing comma if present */
    size_t len = strlen(temp);
    if (len > 0 && temp[len - 1] == ',') {
        temp[len - 1] = '\0';
    }

    for (int i = 0; temp[i]; i++) {
        temp[i] = (char)toupper((unsigned char)temp[i]);
    }

    if (strcmp(temp, "AREG") == 0) return 1;
    if (strcmp(temp, "BREG") == 0) return 2;
    if (strcmp(temp, "CREG") == 0) return 3;
    if (strcmp(temp, "DREG") == 0) return 4;

    /* Condition codes for BC */
    if (strcmp(temp, "LT") == 0) return 1;
    if (strcmp(temp, "LE") == 0) return 2;
    if (strcmp(temp, "EQ") == 0) return 3;
    if (strcmp(temp, "GT") == 0) return 4;
    if (strcmp(temp, "GE") == 0) return 5;
    if (strcmp(temp, "ANY") == 0) return 6;

    return 0;
}

int find_asm_symbol(const AsmSymbolTable *st, const char *name) {
    if (!st || !name) return -1;
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->symbols[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int add_asm_symbol(AsmSymbolTable *st, const char *name, int address) {
    if (!st || !name || *name == '\0') return -1;
    int idx = find_asm_symbol(st, name);
    if (idx != -1) {
        if (address != -1) {
            if (st->symbols[idx].defined) {
                printf("\nError: Duplicate symbol definition '%s'\n", name);
                return -1;
            }
            st->symbols[idx].address = address;
            st->symbols[idx].defined = 1;
        }
        return idx;
    }

    if (st->count >= MAX_SYMBOLS_ASM) {
        printf("\nError: Assembler Symbol Table full!\n");
        return -1;
    }

    AsmSymbol *sym = &st->symbols[st->count];
    strncpy(sym->name, name, MAX_LABEL_LEN - 1);
    sym->name[MAX_LABEL_LEN - 1] = '\0';
    sym->address = address;
    sym->defined = (address != -1) ? 1 : 0;

    int new_idx = st->count;
    st->count++;
    return new_idx;
}

void display_asm_symbol_table(void) {
    if (!g_pass1_done) {
        printf("\nPass 1 has not been executed yet. Run Pass 1 first.\n");
        return;
    }

    printf("\n========================================\n");
    printf("           SYMBOL TABLE\n");
    printf("========================================\n\n");
    printf("%-6s %-20s %-10s\n", "No.", "Symbol", "Address");
    printf("----------------------------------------\n");

    for (int i = 0; i < g_asm_symtab.count; i++) {
        const AsmSymbol *s = &g_asm_symtab.symbols[i];
        if (s->defined) {
            printf("%-6d %-20s %-10d\n", i + 1, s->name, s->address);
        } else {
            printf("%-6d %-20s %-10s\n", i + 1, s->name, "UNDEFINED");
        }
    }
    printf("----------------------------------------\n");
    printf("Total Symbols: %d\n", g_asm_symtab.count);
}

void display_intermediate_code(void) {
    if (!g_pass1_done) {
        printf("\nPass 1 has not been executed yet. Run Pass 1 first.\n");
        return;
    }

    printf("\n========================================\n");
    printf("         INTERMEDIATE CODE\n");
    printf("========================================\n\n");
    printf("%-8s %-15s %-10s %-12s %-25s\n", "Address", "Statement", "Register", "Operand", "Source Line");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < g_inter_code.count; i++) {
        const IntermediateEntry *ie = &g_inter_code.entries[i];

        char addr_str[16];
        if (ie->address >= 0) {
            snprintf(addr_str, sizeof(addr_str), "%d", ie->address);
        } else {
            snprintf(addr_str, sizeof(addr_str), "--");
        }

        char stmt_str[32];
        snprintf(stmt_str, sizeof(stmt_str), "(%s,%02d)", ie->statement_type, ie->opcode);

        char reg_str[16] = "--";
        if (ie->reg_code > 0) {
            snprintf(reg_str, sizeof(reg_str), "(RG,%d)", ie->reg_code);
        }

        char op_str[32] = "--";
        if (strlen(ie->operand_type) > 0) {
            snprintf(op_str, sizeof(op_str), "(%s,%d)", ie->operand_type, ie->operand_val);
        }

        printf("%-8s %-15s %-10s %-12s %-25s\n", addr_str, stmt_str, reg_str, op_str, ie->raw_line);
    }
    printf("----------------------------------------------------------------------\n");
}

void display_object_code(void) {
    if (!g_pass2_done) {
        printf("\nPass 2 has not been executed yet. Run Pass 2 first.\n");
        return;
    }

    printf("\n========================================\n");
    printf("            OBJECT CODE\n");
    printf("========================================\n\n");
    printf("%-8s %-12s %-25s\n", "Address", "Object Code", "Source Line");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < g_obj_code.count; i++) {
        const ObjectCodeEntry *oe = &g_obj_code.entries[i];

        char obj_str[32];
        if (strcmp(oe->type, "IS") == 0) {
            snprintf(obj_str, sizeof(obj_str), "%02d  %d  %03d", oe->opcode, oe->reg_code, oe->operand_address);
        } else if (strcmp(oe->type, "DL") == 0) {
            snprintf(obj_str, sizeof(obj_str), "00  0  %03d", oe->operand_address);
        } else if (strcmp(oe->type, "DS") == 0) {
            snprintf(obj_str, sizeof(obj_str), "--  --  --");
        } else {
            snprintf(obj_str, sizeof(obj_str), "AD DIRECTIVE");
        }

        printf("%-8d %-12s %-25s\n", oe->address, obj_str, oe->raw_line);
    }
    printf("----------------------------------------------------------------------\n");
}

int save_asm_symbol_table(const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) return 0;

    fprintf(fp, "========================================\n");
    fprintf(fp, "           SYMBOL TABLE\n");
    fprintf(fp, "========================================\n\n");
    fprintf(fp, "%-6s %-20s %-10s\n", "No.", "Symbol", "Address");
    fprintf(fp, "----------------------------------------\n");

    for (int i = 0; i < g_asm_symtab.count; i++) {
        const AsmSymbol *s = &g_asm_symtab.symbols[i];
        if (s->defined) {
            fprintf(fp, "%-6d %-20s %-10d\n", i + 1, s->name, s->address);
        } else {
            fprintf(fp, "%-6d %-20s %-10s\n", i + 1, s->name, "UNDEFINED");
        }
    }
    fprintf(fp, "----------------------------------------\n");
    fprintf(fp, "Total Symbols: %d\n", g_asm_symtab.count);

    fclose(fp);
    return 1;
}

int save_intermediate_code(const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) return 0;

    fprintf(fp, "========================================\n");
    fprintf(fp, "         INTERMEDIATE CODE\n");
    fprintf(fp, "========================================\n\n");
    fprintf(fp, "%-8s %-15s %-10s %-12s %-25s\n", "Address", "Statement", "Register", "Operand", "Source Line");
    fprintf(fp, "----------------------------------------------------------------------\n");

    for (int i = 0; i < g_inter_code.count; i++) {
        const IntermediateEntry *ie = &g_inter_code.entries[i];

        char addr_str[16];
        if (ie->address >= 0) {
            snprintf(addr_str, sizeof(addr_str), "%d", ie->address);
        } else {
            snprintf(addr_str, sizeof(addr_str), "--");
        }

        char stmt_str[32];
        snprintf(stmt_str, sizeof(stmt_str), "(%s,%02d)", ie->statement_type, ie->opcode);

        char reg_str[16] = "--";
        if (ie->reg_code > 0) {
            snprintf(reg_str, sizeof(reg_str), "(RG,%d)", ie->reg_code);
        }

        char op_str[32] = "--";
        if (strlen(ie->operand_type) > 0) {
            snprintf(op_str, sizeof(op_str), "(%s,%d)", ie->operand_type, ie->operand_val);
        }

        fprintf(fp, "%-8s %-15s %-10s %-12s %-25s\n", addr_str, stmt_str, reg_str, op_str, ie->raw_line);
    }
    fprintf(fp, "----------------------------------------------------------------------\n");

    fclose(fp);
    return 1;
}

int save_object_code(const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) return 0;

    fprintf(fp, "========================================\n");
    fprintf(fp, "            OBJECT CODE\n");
    fprintf(fp, "========================================\n\n");
    fprintf(fp, "%-8s %-12s %-25s\n", "Address", "Object Code", "Source Line");
    fprintf(fp, "----------------------------------------------------------------------\n");

    for (int i = 0; i < g_obj_code.count; i++) {
        const ObjectCodeEntry *oe = &g_obj_code.entries[i];

        char obj_str[32];
        if (strcmp(oe->type, "IS") == 0) {
            snprintf(obj_str, sizeof(obj_str), "%02d  %d  %03d", oe->opcode, oe->reg_code, oe->operand_address);
        } else if (strcmp(oe->type, "DL") == 0) {
            snprintf(obj_str, sizeof(obj_str), "00  0  %03d", oe->operand_address);
        } else if (strcmp(oe->type, "DS") == 0) {
            snprintf(obj_str, sizeof(obj_str), "--  --  --");
        } else {
            snprintf(obj_str, sizeof(obj_str), "AD DIRECTIVE");
        }

        fprintf(fp, "%-8d %-12s %-25s\n", oe->address, obj_str, oe->raw_line);
    }
    fprintf(fp, "----------------------------------------------------------------------\n");

    fclose(fp);
    return 1;
}

void run_two_pass_assembler(void) {
    int choice = 0;

    while (1) {
        printf("\n========================================\n");
        printf("          TWO PASS ASSEMBLER\n");
        printf("========================================\n\n");
        printf("1. Run Pass 1\n");
        printf("2. Run Pass 2\n");
        printf("3. Run Complete Two-Pass Assembly\n");
        printf("4. Display Symbol Table\n");
        printf("5. Display Intermediate Code\n");
        printf("6. Display Object Code\n");
        printf("7. Back to Main Menu\n\n");
        printf("Enter your choice: ");
        fflush(stdout);

        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid input! Please enter a number between 1 and 7.\n");
            clear_input_buffer();
            press_enter_to_continue();
            continue;
        }

        clear_input_buffer();

        if (choice == 7) {
            break;
        }

        switch (choice) {
            case 1: {
                printf("\n--- Executing Pass 1 ---\n");
                if (execute_pass1("input/assembler_input.asm")) {
                    display_asm_symbol_table();
                    display_intermediate_code();
                    save_asm_symbol_table("output/assembler_symbol_table.txt");
                    save_intermediate_code("output/intermediate_code.txt");
                    printf("\nPass 1 finished successfully!\n");
                    printf("Outputs saved to output/assembler_symbol_table.txt & output/intermediate_code.txt\n");
                }
                press_enter_to_continue();
                break;
            }

            case 2: {
                if (!g_pass1_done) {
                    printf("\nPass 1 has not been run yet. Running Pass 1 first...\n");
                    if (!execute_pass1("input/assembler_input.asm")) {
                        press_enter_to_continue();
                        break;
                    }
                }
                printf("\n--- Executing Pass 2 ---\n");
                if (execute_pass2()) {
                    display_object_code();
                    save_object_code("output/object_code.txt");
                    printf("\nPass 2 finished successfully!\n");
                    printf("Output saved to output/object_code.txt\n");
                }
                press_enter_to_continue();
                break;
            }

            case 3: {
                printf("\n================ PASS 1 ================\n");
                if (!execute_pass1("input/assembler_input.asm")) {
                    printf("\nPass 1 failed.\n");
                    press_enter_to_continue();
                    break;
                }
                display_asm_symbol_table();
                display_intermediate_code();
                save_asm_symbol_table("output/assembler_symbol_table.txt");
                save_intermediate_code("output/intermediate_code.txt");

                printf("\n================ PASS 2 ================\n");
                if (!execute_pass2()) {
                    printf("\nPass 2 failed.\n");
                    press_enter_to_continue();
                    break;
                }
                display_object_code();
                save_object_code("output/object_code.txt");

                printf("\n=========================================\n");
                printf("       ASSEMBLY COMPLETED\n");
                printf("=========================================\n");
                printf("Saved:\n");
                printf(" - output/assembler_symbol_table.txt\n");
                printf(" - output/intermediate_code.txt\n");
                printf(" - output/object_code.txt\n");
                press_enter_to_continue();
                break;
            }

            case 4:
                display_asm_symbol_table();
                press_enter_to_continue();
                break;

            case 5:
                display_intermediate_code();
                press_enter_to_continue();
                break;

            case 6:
                display_object_code();
                press_enter_to_continue();
                break;

            default:
                printf("\nInvalid choice! Please select an option between 1 and 7.\n");
                press_enter_to_continue();
                break;
        }
    }
}
