#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "../common/common.h"

int execute_pass2(void) {
    if (!g_pass1_done) {
        printf("\nError: Pass 1 must be executed before Pass 2.\n");
        return 0;
    }

    /* Check for undefined symbols */
    int undefined_found = 0;
    for (int i = 0; i < g_asm_symtab.count; i++) {
        if (!g_asm_symtab.symbols[i].defined) {
            printf("\nError in Pass 2: Undefined symbol '%s' used in program.\n", g_asm_symtab.symbols[i].name);
            undefined_found = 1;
        }
    }

    if (undefined_found) {
        printf("\nPass 2 aborted due to undefined symbol errors.\n");
        return 0;
    }

    g_obj_code.count = 0;

    for (int i = 0; i < g_inter_code.count; i++) {
        const IntermediateEntry *ie = &g_inter_code.entries[i];

        /* Skip AD directives like START / END from object code generation */
        if (strcmp(ie->statement_type, "AD") == 0) {
            continue;
        }

        if (g_obj_code.count >= MAX_ASM_LINES) {
            printf("\nError: Object code buffer full!\n");
            break;
        }

        ObjectCodeEntry *oe = &g_obj_code.entries[g_obj_code.count];
        memset(oe, 0, sizeof(ObjectCodeEntry));

        oe->address = ie->address;
        strncpy(oe->raw_line, ie->raw_line, sizeof(oe->raw_line) - 1);
        strncpy(oe->type, ie->statement_type, sizeof(oe->type) - 1);

        if (strcmp(ie->statement_type, "IS") == 0) {
            oe->opcode = ie->opcode;
            oe->reg_code = ie->reg_code;

            if (strcmp(ie->operand_type, "S") == 0) {
                int sym_idx = find_asm_symbol(&g_asm_symtab, ie->operand_name);
                if (sym_idx != -1) {
                    oe->operand_address = g_asm_symtab.symbols[sym_idx].address;
                } else {
                    printf("\nError in Pass 2: Unresolved symbol '%s' at address %d\n", ie->operand_name, ie->address);
                    oe->operand_address = 0;
                }
            } else if (strcmp(ie->operand_type, "C") == 0) {
                oe->operand_address = ie->operand_val;
            }
        } else if (strcmp(ie->statement_type, "DL") == 0) {
            if (ie->opcode == 1) {
                /* DC directive */
                strncpy(oe->type, "DL", sizeof(oe->type) - 1);
                oe->opcode = 0;
                oe->reg_code = 0;
                oe->operand_address = ie->operand_val;
            } else if (ie->opcode == 2) {
                /* DS directive */
                strncpy(oe->type, "DS", sizeof(oe->type) - 1);
                oe->opcode = 0;
                oe->reg_code = 0;
                oe->operand_address = ie->operand_val;
            }
        }

        g_obj_code.count++;
    }

    g_pass2_done = 1;
    return 1;
}
