#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "macro_processor.h"
#include "../common/common.h"

static MNT g_mnt;
static MDT g_mdt;
static ExpandedProgram g_expanded;
static int g_macro_processed = 0;

void init_macro_processor_state(void) {
    g_mnt.count = 0;
    g_mdt.count = 0;
    g_expanded.count = 0;
    g_macro_processed = 0;
}

static void trim_line(char *line) {
    /* Remove trailing carriage return/newline/whitespace */
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || isspace((unsigned char)line[len - 1]))) {
        line[len - 1] = '\0';
        len--;
    }
}

static void replace_parameter(const char *template_str, const char *formal, const char *actual, char *result, size_t result_size) {
    result[0] = '\0';
    const char *pos = template_str;
    const char *found;
    size_t formal_len = strlen(formal);

    while ((found = strstr(pos, formal)) != NULL) {
        size_t len_before = found - pos;
        if (strlen(result) + len_before < result_size) {
            strncat(result, pos, len_before);
        }
        if (strlen(result) + strlen(actual) < result_size) {
            strcat(result, actual);
        }
        pos = found + formal_len;
    }
    if (strlen(result) + strlen(pos) < result_size) {
        strcat(result, pos);
    }
}

static int find_macro_in_mnt(const char *name) {
    for (int i = 0; i < g_mnt.count; i++) {
        if (strcmp(g_mnt.entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int process_macros(const char *input_path) {
    init_macro_processor_state();

    FILE *fp = fopen(input_path, "r");
    if (!fp) {
        printf("\nError: Input file '%s' not found.\n", input_path);
        return 0;
    }

    char line[MAX_LINE_LEN];
    char temp_line[MAX_LINE_LEN];
    int line_num = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        strncpy(temp_line, line, sizeof(temp_line) - 1);
        temp_line[sizeof(temp_line) - 1] = '\0';
        trim_line(temp_line);

        /* Skip empty lines */
        char *ptr = temp_line;
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (*ptr == '\0') continue;

        /* Check if line is MACRO keyword */
        if (strcmp(ptr, "MACRO") == 0) {
            int def_start_line = line_num;
            /* Read next line for Macro Header (Name + Parameters) */
            if (!fgets(line, sizeof(line), fp)) {
                printf("\nError: Unexpected end of file after MACRO directive at line %d\n", def_start_line);
                fclose(fp);
                return 0;
            }
            line_num++;
            strncpy(temp_line, line, sizeof(temp_line) - 1);
            temp_line[sizeof(temp_line) - 1] = '\0';
            trim_line(temp_line);

            char header_copy[MAX_LINE_LEN];
            strncpy(header_copy, temp_line, sizeof(header_copy) - 1);
            header_copy[sizeof(header_copy) - 1] = '\0';

            char macro_name[MAX_NAME_LEN] = {0};
            char param_names[MAX_PARAMS][MAX_NAME_LEN];
            int param_count = 0;

            char *tok = strtok(header_copy, " \t,");
            if (tok) {
                strncpy(macro_name, tok, sizeof(macro_name) - 1);
                while ((tok = strtok(NULL, " \t,")) != NULL) {
                    if (param_count < MAX_PARAMS) {
                        strncpy(param_names[param_count], tok, MAX_NAME_LEN - 1);
                        param_names[param_count][MAX_NAME_LEN - 1] = '\0';
                        param_count++;
                    }
                }
            }

            if (strlen(macro_name) == 0) {
                printf("\nError at line %d: Invalid macro header definition.\n", line_num);
                fclose(fp);
                return 0;
            }

            if (find_macro_in_mnt(macro_name) != -1) {
                printf("\nError at line %d: Duplicate macro definition '%s'\n", line_num, macro_name);
                fclose(fp);
                return 0;
            }

            /* Record MNT Entry */
            int mnt_idx = g_mnt.count;
            g_mnt.entries[mnt_idx].index = mnt_idx + 1;
            strncpy(g_mnt.entries[mnt_idx].name, macro_name, MAX_NAME_LEN - 1);
            g_mnt.entries[mnt_idx].mdt_index = g_mdt.count + 1;
            g_mnt.entries[mnt_idx].param_count = param_count;

            for (int k = 0; k < param_count; k++) {
                strncpy(g_mnt.entries[mnt_idx].param_names[k], param_names[k], MAX_NAME_LEN - 1);
            }
            g_mnt.count++;

            /* Read Macro Body until MEND */
            int mend_found = 0;
            while (fgets(line, sizeof(line), fp)) {
                line_num++;
                strncpy(temp_line, line, sizeof(temp_line) - 1);
                temp_line[sizeof(temp_line) - 1] = '\0';
                trim_line(temp_line);

                char *b_ptr = temp_line;
                while (*b_ptr && isspace((unsigned char)*b_ptr)) b_ptr++;

                if (strcmp(b_ptr, "MEND") == 0) {
                    mend_found = 1;
                    g_mnt.entries[mnt_idx].mdt_end_index = g_mdt.count;
                    break;
                }

                if (g_mdt.count < MAX_MDT_ENTRIES) {
                    g_mdt.entries[g_mdt.count].index = g_mdt.count + 1;
                    strncpy(g_mdt.entries[g_mdt.count].statement, b_ptr, MAX_LINE_LEN - 1);
                    g_mdt.count++;
                }
            }

            if (!mend_found) {
                printf("\nError: MACRO definition for '%s' starting at line %d is missing 'MEND'\n", macro_name, def_start_line);
                fclose(fp);
                return 0;
            }

            continue;
        }

        /* Outside macro definition: check if line is a Macro Call or regular instruction */
        char call_copy[MAX_LINE_LEN];
        strncpy(call_copy, ptr, sizeof(call_copy) - 1);
        call_copy[sizeof(call_copy) - 1] = '\0';

        char call_name[MAX_NAME_LEN] = {0};
        char actual_args[MAX_PARAMS][MAX_NAME_LEN];
        int arg_count = 0;

        char *tok = strtok(call_copy, " \t,");
        if (tok) {
            strncpy(call_name, tok, sizeof(call_name) - 1);
            while ((tok = strtok(NULL, " \t,")) != NULL) {
                if (arg_count < MAX_PARAMS) {
                    strncpy(actual_args[arg_count], tok, MAX_NAME_LEN - 1);
                    actual_args[arg_count][MAX_NAME_LEN - 1] = '\0';
                    arg_count++;
                }
            }
        }

        int m_idx = find_macro_in_mnt(call_name);
        if (m_idx != -1) {
            /* Macro Invocation! Expand macro body from MDT */
            MNTEntry *ment = &g_mnt.entries[m_idx];

            for (int j = ment->mdt_index - 1; j < ment->mdt_end_index; j++) {
                char current_stmt[MAX_LINE_LEN];
                strncpy(current_stmt, g_mdt.entries[j].statement, sizeof(current_stmt) - 1);
                current_stmt[sizeof(current_stmt) - 1] = '\0';

                char exp_stmt[MAX_LINE_LEN];
                char temp_exp[MAX_LINE_LEN];
                strncpy(exp_stmt, current_stmt, sizeof(exp_stmt) - 1);
                exp_stmt[sizeof(exp_stmt) - 1] = '\0';

                /* Substitute formal parameters with actual parameters */
                for (int k = 0; k < ment->param_count && k < arg_count; k++) {
                    replace_parameter(exp_stmt, ment->param_names[k], actual_args[k], temp_exp, sizeof(temp_exp));
                    strncpy(exp_stmt, temp_exp, sizeof(exp_stmt) - 1);
                    exp_stmt[sizeof(exp_stmt) - 1] = '\0';
                }

                if (g_expanded.count < MAX_EXP_LINES) {
                    strncpy(g_expanded.lines[g_expanded.count], exp_stmt, MAX_LINE_LEN - 1);
                    g_expanded.count++;
                }
            }
        } else {
            /* Regular Assembly Line: Pass through */
            if (g_expanded.count < MAX_EXP_LINES) {
                strncpy(g_expanded.lines[g_expanded.count], ptr, MAX_LINE_LEN - 1);
                g_expanded.count++;
            }
        }
    }

    fclose(fp);
    g_macro_processed = 1;
    return 1;
}

void display_mnt(void) {
    if (!g_macro_processed) {
        printf("\nMacro Processor has not been run yet.\n");
        return;
    }

    printf("\n--- MACRO NAME TABLE (MNT) ---\n");
    printf("----------------------------------------------------------------------\n");
    printf("%-8s %-20s %-12s %-12s %-15s\n", "Index", "Macro Name", "MDT Index", "Parameters", "Formal Params");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < g_mnt.count; i++) {
        const MNTEntry *e = &g_mnt.entries[i];
        char params_str[64] = "";
        for (int k = 0; k < e->param_count; k++) {
            if (k > 0) strcat(params_str, ", ");
            strcat(params_str, e->param_names[k]);
        }
        printf("%-8d %-20s %-12d %-12d %-15s\n", e->index, e->name, e->mdt_index, e->param_count, params_str);
    }
    printf("----------------------------------------------------------------------\n");
    printf("Total Macros: %d\n", g_mnt.count);
}

void display_mdt(void) {
    if (!g_macro_processed) {
        printf("\nMacro Processor has not been run yet.\n");
        return;
    }

    printf("\n--- MACRO DEFINITION TABLE (MDT) ---\n");
    printf("----------------------------------------------------------------------\n");
    printf("%-8s %-40s\n", "Index", "Macro Definition Statement");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < g_mdt.count; i++) {
        const MDTEntry *e = &g_mdt.entries[i];
        printf("%-8d %-40s\n", e->index, e->statement);
    }
    printf("----------------------------------------------------------------------\n");
    printf("Total Statements: %d\n", g_mdt.count);
}

void display_expanded_program(void) {
    if (!g_macro_processed) {
        printf("\nMacro Processor has not been run yet.\n");
        return;
    }

    printf("\n--- EXPANDED PROGRAM ---\n");
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < g_expanded.count; i++) {
        printf("%s\n", g_expanded.lines[i]);
    }
    printf("----------------------------------------------------------------------\n");
    printf("Total Expanded Lines: %d\n", g_expanded.count);
}

int save_macro_output(const char *output_path) {
    FILE *fp = fopen(output_path, "w");
    if (!fp) {
        printf("\nError: Could not open output file '%s'\n", output_path);
        return 0;
    }

    fprintf(fp, "========================================\n");
    fprintf(fp, "        MACRO PROCESSOR OUTPUT\n");
    fprintf(fp, "========================================\n\n");

    fprintf(fp, "--- MACRO NAME TABLE (MNT) ---\n");
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "%-8s %-20s %-12s %-12s %-15s\n", "Index", "Macro Name", "MDT Index", "Parameters", "Formal Params");
    fprintf(fp, "----------------------------------------------------------------------\n");
    for (int i = 0; i < g_mnt.count; i++) {
        const MNTEntry *e = &g_mnt.entries[i];
        char params_str[64] = "";
        for (int k = 0; k < e->param_count; k++) {
            if (k > 0) strcat(params_str, ", ");
            strcat(params_str, e->param_names[k]);
        }
        fprintf(fp, "%-8d %-20s %-12d %-12d %-15s\n", e->index, e->name, e->mdt_index, e->param_count, params_str);
    }
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "Total Macros: %d\n\n", g_mnt.count);

    fprintf(fp, "--- MACRO DEFINITION TABLE (MDT) ---\n");
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "%-8s %-40s\n", "Index", "Macro Definition Statement");
    fprintf(fp, "----------------------------------------------------------------------\n");
    for (int i = 0; i < g_mdt.count; i++) {
        const MDTEntry *e = &g_mdt.entries[i];
        fprintf(fp, "%-8d %-40s\n", e->index, e->statement);
    }
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "Total Statements: %d\n\n", g_mdt.count);

    fprintf(fp, "--- EXPANDED PROGRAM ---\n");
    fprintf(fp, "----------------------------------------------------------------------\n");
    for (int i = 0; i < g_expanded.count; i++) {
        fprintf(fp, "%s\n", g_expanded.lines[i]);
    }
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "Total Expanded Lines: %d\n", g_expanded.count);

    fclose(fp);
    return 1;
}

void run_macro_processor(void) {
    int choice = 0;

    while (1) {
        printf("\n========================================\n");
        printf("          MACRO PROCESSOR\n");
        printf("========================================\n\n");
        printf("1. Run Macro Processor\n");
        printf("2. Display MNT\n");
        printf("3. Display MDT\n");
        printf("4. Display Expanded Program\n");
        printf("5. Back to Main Menu\n\n");
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
            case 1:
                printf("\nProcessing 'input/macro_input.asm'...\n");
                if (process_macros("input/macro_input.asm")) {
                    printf("\n========================================\n");
                    printf("        MACRO PROCESSOR COMPLETED\n");
                    printf("========================================\n");
                    display_mnt();
                    display_mdt();
                    display_expanded_program();

                    if (save_macro_output("output/macro_output.txt")) {
                        printf("\nAll results saved to output/macro_output.txt\n");
                    }
                }
                press_enter_to_continue();
                break;

            case 2:
                display_mnt();
                press_enter_to_continue();
                break;

            case 3:
                display_mdt();
                press_enter_to_continue();
                break;

            case 4:
                display_expanded_program();
                press_enter_to_continue();
                break;

            default:
                printf("\nInvalid choice! Please select an option between 1 and 5.\n");
                press_enter_to_continue();
                break;
        }
    }
}
