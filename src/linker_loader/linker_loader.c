#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "linker_loader.h"
#include "../common/common.h"

static void trim_line(char *line) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || isspace((unsigned char)line[len - 1]))) {
        line[len - 1] = '\0';
        len--;
    }
}

static int parse_modules(const char *input_text, Module *modules, int max_modules, char *error_msg, size_t err_size) {
    int module_count = 0;
    Module *curr_mod = NULL;

    char line_buf[MAX_LINE_LEN];
    const char *ptr = input_text;

    while (*ptr != '\0' && module_count <= max_modules) {
        int line_pos = 0;
        while (*ptr != '\0' && *ptr != '\n' && line_pos < MAX_LINE_LEN - 1) {
            line_buf[line_pos++] = *ptr++;
        }
        if (*ptr == '\n') ptr++;
        line_buf[line_pos] = '\0';
        trim_line(line_buf);

        char *start_ptr = line_buf;
        while (*start_ptr && isspace((unsigned char)*start_ptr)) start_ptr++;
        if (*start_ptr == '\0') continue;

        char tokens[4][MAX_NAME_LEN] = {{0}};
        int token_count = 0;
        char line_copy[MAX_LINE_LEN];
        strncpy(line_copy, start_ptr, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';

        char *tok = strtok(line_copy, " \t");
        while (tok && token_count < 4) {
            strncpy(tokens[token_count], tok, MAX_NAME_LEN - 1);
            tokens[token_count][MAX_NAME_LEN - 1] = '\0';
            token_count++;
            tok = strtok(NULL, " \t");
        }

        if (token_count == 0) continue;

        if (strcmp(tokens[0], "MODULE") == 0) {
            if (token_count < 2) {
                snprintf(error_msg, err_size, "ERROR: Missing module name after MODULE directive");
                return -1;
            }
            if (module_count >= max_modules) {
                snprintf(error_msg, err_size, "ERROR: Maximum modules limit reached");
                return -1;
            }
            curr_mod = &modules[module_count];
            memset(curr_mod, 0, sizeof(Module));
            strncpy(curr_mod->name, tokens[1], MAX_NAME_LEN - 1);
            module_count++;
        } else if (strcmp(tokens[0], "START") == 0) {
            if (!curr_mod) {
                snprintf(error_msg, err_size, "ERROR: START directive outside MODULE block");
                return -1;
            }
            if (token_count > 1) {
                curr_mod->start_address = atoi(tokens[1]);
            }
        } else if (strcmp(tokens[0], "DEF") == 0) {
            if (!curr_mod) {
                snprintf(error_msg, err_size, "ERROR: DEF directive outside MODULE block");
                return -1;
            }
            if (token_count < 3) {
                snprintf(error_msg, err_size, "ERROR: DEF requires symbol name and address");
                return -1;
            }
            if (curr_mod->def_count < MAX_EST_SYMBOLS) {
                DefSymbol *ds = &curr_mod->def_symbols[curr_mod->def_count];
                strncpy(ds->name, tokens[1], MAX_NAME_LEN - 1);
                ds->address = atoi(tokens[2]);
                curr_mod->def_count++;
            }
        } else if (strcmp(tokens[0], "EXT") == 0) {
            if (!curr_mod) {
                snprintf(error_msg, err_size, "ERROR: EXT directive outside MODULE block");
                return -1;
            }
            if (token_count < 2) {
                snprintf(error_msg, err_size, "ERROR: EXT requires symbol name");
                return -1;
            }
            if (curr_mod->ext_count < MAX_EST_SYMBOLS) {
                strncpy(curr_mod->ext_symbols[curr_mod->ext_count], tokens[1], MAX_NAME_LEN - 1);
                curr_mod->ext_count++;
            }
        } else if (strcmp(tokens[0], "END") == 0) {
            curr_mod = NULL;
        } else if (isdigit((unsigned char)tokens[0][0])) {
            if (!curr_mod) {
                snprintf(error_msg, err_size, "ERROR: Memory record outside MODULE block");
                return -1;
            }
            if (curr_mod->record_count < MAX_MEM_RECORDS) {
                ModuleRecord *rec = &curr_mod->records[curr_mod->record_count];
                rec->address = atoi(tokens[0]);
                rec->value = (token_count > 1) ? atoi(tokens[1]) : 0;
                if (token_count > 2) {
                    strncpy(rec->symbol_ref, tokens[2], MAX_NAME_LEN - 1);
                } else {
                    rec->symbol_ref[0] = '\0';
                }
                curr_mod->record_count++;
            }
        }
    }

    return module_count;
}

static int pass1_symbol_collection(LinkerResult *res) {
    res->est.count = 0;

    for (int m = 0; m < res->module_count; m++) {
        const Module *mod = &res->modules[m];

        for (int d = 0; d < mod->def_count; d++) {
            const DefSymbol *def = &mod->def_symbols[d];

            for (int e = 0; e < res->est.count; e++) {
                if (strcmp(res->est.entries[e].name, def->name) == 0) {
                    snprintf(res->error_msg, sizeof(res->error_msg), "ERROR: Duplicate external symbol '%s'", def->name);
                    return 0;
                }
            }

            if (res->est.count < MAX_EST_SYMBOLS) {
                ESTEntry *entry = &res->est.entries[res->est.count];
                strncpy(entry->name, def->name, MAX_NAME_LEN - 1);
                strncpy(entry->module_name, mod->name, MAX_NAME_LEN - 1);
                entry->relative_address = def->address;
                entry->linked_address = def->address;
                res->est.count++;
            }
        }
    }

    return 1;
}

static int pass2_link_and_load(LinkerResult *res) {
    res->memory_map.count = 0;

    for (int m = 0; m < res->module_count; m++) {
        const Module *mod = &res->modules[m];

        for (int x = 0; x < mod->ext_count; x++) {
            const char *ext_name = mod->ext_symbols[x];
            int found = 0;
            for (int e = 0; e < res->est.count; e++) {
                if (strcmp(res->est.entries[e].name, ext_name) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                snprintf(res->error_msg, sizeof(res->error_msg), "ERROR: Undefined external symbol '%s'", ext_name);
                return 0;
            }
        }
    }

    for (int m = 0; m < res->module_count; m++) {
        const Module *mod = &res->modules[m];

        for (int r = 0; r < mod->record_count; r++) {
            const ModuleRecord *rec = &mod->records[r];

            if (res->memory_map.count >= MAX_MEM_RECORDS) {
                snprintf(res->error_msg, sizeof(res->error_msg), "ERROR: Memory map limit reached");
                return 0;
            }

            LinkedMemoryEntry *lme = &res->memory_map.entries[res->memory_map.count];
            memset(lme, 0, sizeof(LinkedMemoryEntry));

            lme->address = rec->address;
            lme->value = rec->value;
            strncpy(lme->module_name, mod->name, MAX_NAME_LEN - 1);

            if (strlen(rec->symbol_ref) > 0) {
                strncpy(lme->symbol_ref, rec->symbol_ref, MAX_NAME_LEN - 1);
                lme->resolved_addr = -1;

                for (int e = 0; e < res->est.count; e++) {
                    if (strcmp(res->est.entries[e].name, rec->symbol_ref) == 0) {
                        lme->resolved_addr = res->est.entries[e].linked_address;
                        break;
                    }
                }
            } else {
                lme->symbol_ref[0] = '\0';
                lme->resolved_addr = -1;
            }

            res->memory_map.count++;
        }
    }

    return 1;
}

int link_and_load(const char *input_text, LinkerResult *result) {
    if (!result) return 0;
    memset(result, 0, sizeof(LinkerResult));

    if (!input_text || strlen(input_text) == 0) {
        result->is_valid = 0;
        strncpy(result->error_msg, "ERROR: Empty input", sizeof(result->error_msg) - 1);
        result->executed = 1;
        return 0;
    }

    int count = parse_modules(input_text, result->modules, MAX_MODULES, result->error_msg, sizeof(result->error_msg));
    if (count <= 0) {
        result->is_valid = 0;
        if (strlen(result->error_msg) == 0) {
            strncpy(result->error_msg, "ERROR: No valid modules found", sizeof(result->error_msg) - 1);
        }
        result->executed = 1;
        return 0;
    }
    result->module_count = count;

    if (!pass1_symbol_collection(result)) {
        result->is_valid = 0;
        result->executed = 1;
        return 0;
    }

    if (!pass2_link_and_load(result)) {
        result->is_valid = 0;
        result->executed = 1;
        return 0;
    }

    result->is_valid = 1;
    result->executed = 1;
    return 1;
}

void display_est(const LinkerResult *result) {
    if (!result || !result->executed) {
        printf("\nNo Linker Loader result available. Run option 1 first.\n");
        return;
    }

    if (!result->is_valid) {
        printf("\n%s\n", result->error_msg);
        return;
    }

    printf("\n--- EXTERNAL SYMBOL TABLE (EST) ---\n");
    printf("----------------------------------------------------------------------\n");
    printf("%-15s %-15s %-18s %-15s\n", "Symbol", "Module", "Relative Addr", "Linked Addr");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < result->est.count; i++) {
        const ESTEntry *e = &result->est.entries[i];
        printf("%-15s %-15s %-18d %-15d\n", e->name, e->module_name, e->relative_address, e->linked_address);
    }
    printf("----------------------------------------------------------------------\n");
    printf("Total External Symbols: %d\n", result->est.count);
}

void display_memory_map(const LinkerResult *result) {
    if (!result || !result->executed) {
        printf("\nNo Linker Loader result available. Run option 1 first.\n");
        return;
    }

    if (!result->is_valid) {
        printf("\n%s\n", result->error_msg);
        return;
    }

    printf("\n--- LINKED MEMORY MAP ---\n");
    printf("----------------------------------------------------------------------\n");
    printf("%-10s %-15s %-20s %-12s\n", "Address", "Object Code", "Symbol / Ref", "Module");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < result->memory_map.count; i++) {
        const LinkedMemoryEntry *m = &result->memory_map.entries[i];

        char sym_str[32] = "--";
        if (strlen(m->symbol_ref) > 0) {
            if (m->resolved_addr != -1) {
                snprintf(sym_str, sizeof(sym_str), "%s (%d)", m->symbol_ref, m->resolved_addr);
            } else {
                snprintf(sym_str, sizeof(sym_str), "%s", m->symbol_ref);
            }
        }

        printf("%-10d %-15d %-20s %-12s\n", m->address, m->value, sym_str, m->module_name);
    }
    printf("----------------------------------------------------------------------\n");
    printf("Total Memory Locations Loaded: %d\n", result->memory_map.count);
}

int save_linker_output(const LinkerResult *result, const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        printf("\nError: Could not open output file '%s'\n", filepath);
        return 0;
    }

    fprintf(fp, "========================================\n");
    fprintf(fp, "             LINKER LOADER\n");
    fprintf(fp, "========================================\n\n");

    if (!result->is_valid) {
        fprintf(fp, "STATUS:\nLINKING FAILED\n\n");
        fprintf(fp, "%s\n", result->error_msg);
        fprintf(fp, "========================================\n");
        fclose(fp);
        return 1;
    }

    fprintf(fp, "PASS 1 COMPLETED\n\n");
    fprintf(fp, "--- EXTERNAL SYMBOL TABLE (EST) ---\n");
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "%-15s %-15s %-18s %-15s\n", "Symbol", "Module", "Relative Addr", "Linked Addr");
    fprintf(fp, "----------------------------------------------------------------------\n");
    for (int i = 0; i < result->est.count; i++) {
        const ESTEntry *e = &result->est.entries[i];
        fprintf(fp, "%-15s %-15s %-18d %-15d\n", e->name, e->module_name, e->relative_address, e->linked_address);
    }
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "Total External Symbols: %d\n\n", result->est.count);

    fprintf(fp, "PASS 2 COMPLETED\n\n");
    fprintf(fp, "--- LINKED MEMORY MAP ---\n");
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "%-10s %-15s %-20s %-12s\n", "Address", "Object Code", "Symbol / Ref", "Module");
    fprintf(fp, "----------------------------------------------------------------------\n");
    for (int i = 0; i < result->memory_map.count; i++) {
        const LinkedMemoryEntry *m = &result->memory_map.entries[i];

        char sym_str[32] = "--";
        if (strlen(m->symbol_ref) > 0) {
            if (m->resolved_addr != -1) {
                snprintf(sym_str, sizeof(sym_str), "%s (%d)", m->symbol_ref, m->resolved_addr);
            } else {
                snprintf(sym_str, sizeof(sym_str), "%s", m->symbol_ref);
            }
        }

        fprintf(fp, "%-10d %-15d %-20s %-12s\n", m->address, m->value, sym_str, m->module_name);
    }
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "Total Memory Locations Loaded: %d\n\n", result->memory_map.count);

    fprintf(fp, "STATUS:\nLINKING SUCCESSFUL\nLOADING COMPLETED\n");
    fprintf(fp, "========================================\n");

    fclose(fp);
    return 1;
}

void run_linker_loader(void) {
    int choice = 0;
    static LinkerResult last_result;

    while (1) {
        printf("\n========================================\n");
        printf("          LINKER LOADER\n");
        printf("========================================\n");
        printf("1. Link and Load from Input File\n");
        printf("2. Display External Symbol Table\n");
        printf("3. Display Linked Memory Map\n");
        printf("4. Back to Main Menu\n");
        printf("========================================\n\n");
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
                char filepath[256] = "input/linker_input.txt";
                FILE *fp = fopen(filepath, "rb");
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

                if (link_and_load(file_buf, &last_result)) {
                    printf("\nPASS 1 COMPLETED\n");
                    printf("PASS 2 COMPLETED\n");
                    printf("LINKING SUCCESSFUL\n");
                    printf("LOADING COMPLETED\n");
                    display_est(&last_result);
                    display_memory_map(&last_result);

                    if (save_linker_output(&last_result, "output/linker_output.txt")) {
                        printf("\nResults saved to output/linker_output.txt\n");
                    }
                } else {
                    printf("\nLINKING FAILED\n");
                    printf("%s\n", last_result.error_msg);
                    save_linker_output(&last_result, "output/linker_output.txt");
                }

                free(file_buf);
                press_enter_to_continue();
                break;
            }

            case 2:
                display_est(&last_result);
                press_enter_to_continue();
                break;

            case 3:
                display_memory_map(&last_result);
                press_enter_to_continue();
                break;

            default:
                printf("\nInvalid choice! Please select an option between 1 and 4.\n");
                press_enter_to_continue();
                break;
        }
    }
}
