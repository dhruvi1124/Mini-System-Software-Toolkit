#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbol_table.h"
#include "../common/common.h"

void init_symbol_table(SymbolTable *st) {
    if (!st) return;
    st->count = 0;
    st->next_address = 1000;
}

int get_datatype_size(const char *datatype) {
    if (!datatype) return 4;
    if (strcmp(datatype, "char") == 0 || strcmp(datatype, "_Bool") == 0) return 1;
    if (strcmp(datatype, "short") == 0) return 2;
    if (strcmp(datatype, "int") == 0 || strcmp(datatype, "float") == 0) return 4;
    if (strcmp(datatype, "double") == 0 || strcmp(datatype, "long") == 0) return 8;
    return 4;
}

int search_symbol(const SymbolTable *st, const char *name) {
    if (!st || !name) return -1;
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->symbols[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int insert_symbol(SymbolTable *st, const char *name, const char *data_type, const char *scope, int address, int size) {
    if (!st || !name || *name == '\0') return 0;
    if (st->count >= MAX_SYMBOLS) {
        printf("\nError: Symbol Table is full!\n");
        return 0;
    }

    if (search_symbol(st, name) != -1) {
        printf("\nWarning: Symbol '%s' already exists in the table.\n", name);
        return 0;
    }

    Symbol *s = &st->symbols[st->count];
    strncpy(s->name, name, MAX_NAME_LEN - 1);
    s->name[MAX_NAME_LEN - 1] = '\0';

    strncpy(s->data_type, data_type ? data_type : "int", MAX_TYPE_LEN - 1);
    s->data_type[MAX_TYPE_LEN - 1] = '\0';

    strncpy(s->scope, scope ? scope : "global", MAX_SCOPE_LEN - 1);
    s->scope[MAX_SCOPE_LEN - 1] = '\0';

    s->size = (size > 0) ? size : get_datatype_size(s->data_type);

    if (address >= 0) {
        s->address = address;
        if (address + s->size > st->next_address) {
            st->next_address = address + s->size;
        }
    } else {
        s->address = st->next_address;
        st->next_address += s->size;
    }

    st->count++;
    return 1;
}

int delete_symbol(SymbolTable *st, const char *name) {
    int index = search_symbol(st, name);
    if (index == -1) {
        return 0;
    }

    for (int i = index; i < st->count - 1; i++) {
        st->symbols[i] = st->symbols[i + 1];
    }
    st->count--;
    return 1;
}

int update_symbol(SymbolTable *st, const char *name, const char *new_type, const char *new_scope, int new_address, int new_size) {
    int index = search_symbol(st, name);
    if (index == -1) {
        return 0;
    }

    Symbol *s = &st->symbols[index];
    if (new_type && *new_type) {
        strncpy(s->data_type, new_type, MAX_TYPE_LEN - 1);
        s->data_type[MAX_TYPE_LEN - 1] = '\0';
    }
    if (new_scope && *new_scope) {
        strncpy(s->scope, new_scope, MAX_SCOPE_LEN - 1);
        s->scope[MAX_SCOPE_LEN - 1] = '\0';
    }
    if (new_size > 0) {
        s->size = new_size;
    } else if (new_type && *new_type) {
        s->size = get_datatype_size(new_type);
    }
    if (new_address >= 0) {
        s->address = new_address;
    }

    return 1;
}

void display_symbol_table(const SymbolTable *st) {
    if (!st || st->count == 0) {
        printf("\nSymbol Table is empty.\n");
        return;
    }

    printf("\n----------------------------------------------------------------------\n");
    printf("%-6s %-20s %-15s %-12s %-10s %-6s\n", "Index", "Symbol Name", "Data Type", "Scope", "Address", "Size");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < st->count; i++) {
        const Symbol *s = &st->symbols[i];
        printf("%-6d %-20s %-15s %-12s %-10d %-6d\n", i + 1, s->name, s->data_type, s->scope, s->address, s->size);
    }

    printf("----------------------------------------------------------------------\n");
    printf("Total Symbols: %d\n", st->count);
}

int save_symbol_table_to_file(const SymbolTable *st, const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        printf("\nError: Could not open file '%s' for writing.\n", filepath);
        return 0;
    }

    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "%-6s %-20s %-15s %-12s %-10s %-6s\n", "Index", "Symbol Name", "Data Type", "Scope", "Address", "Size");
    fprintf(fp, "----------------------------------------------------------------------\n");

    for (int i = 0; i < st->count; i++) {
        const Symbol *s = &st->symbols[i];
        fprintf(fp, "%-6d %-20s %-15s %-12s %-10d %-6d\n", i + 1, s->name, s->data_type, s->scope, s->address, s->size);
    }

    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "Total Symbols: %d\n", st->count);

    fclose(fp);
    return 1;
}

int load_symbols_from_file(SymbolTable *st, const char *filepath) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("\nError: Could not open file '%s'. Make sure the file exists.\n", filepath);
        return 0;
    }

    char line[256];
    int loaded = 0;

    while (fgets(line, sizeof(line), fp)) {
        char type[MAX_TYPE_LEN] = {0};
        char name[MAX_NAME_LEN] = {0};

        if (sscanf(line, "%19s %49s", type, name) == 2) {
            size_t name_len = strlen(name);
            while (name_len > 0 && (name[name_len - 1] == ';' || name[name_len - 1] == ',')) {
                name[name_len - 1] = '\0';
                name_len--;
            }

            if (strlen(type) > 0 && strlen(name) > 0) {
                int size = get_datatype_size(type);
                if (insert_symbol(st, name, type, "global", -1, size)) {
                    loaded++;
                }
            }
        }
    }

    fclose(fp);
    return loaded;
}

void run_symbol_table(void) {
    static SymbolTable st;
    static int initialized = 0;

    if (!initialized) {
        init_symbol_table(&st);
        initialized = 1;
        load_symbols_from_file(&st, "input/symbol_table_input.txt");
        save_symbol_table_to_file(&st, "output/symbol_table.txt");
    }

    int choice = 0;

    while (1) {
        printf("\n========================================\n");
        printf("            SYMBOL TABLE\n");
        printf("========================================\n\n");
        printf("1. Load Symbols From Input File\n");
        printf("2. Insert Symbol\n");
        printf("3. Search Symbol\n");
        printf("4. Delete Symbol\n");
        printf("5. Update Symbol\n");
        printf("6. Display Symbol Table\n");
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
                char filepath[256];
                printf("\nEnter input file path [Default: input/symbol_table_input.txt]: ");
                fflush(stdout);

                if (!fgets(filepath, sizeof(filepath), stdin)) {
                    strcpy(filepath, "input/symbol_table_input.txt");
                } else {
                    filepath[strcspn(filepath, "\r\n")] = '\0';
                    if (strlen(filepath) == 0) {
                        strcpy(filepath, "input/symbol_table_input.txt");
                    }
                }

                int loaded = load_symbols_from_file(&st, filepath);
                if (loaded > 0) {
                    printf("\nSuccessfully loaded %d symbols from '%s'.\n", loaded, filepath);
                    save_symbol_table_to_file(&st, "output/symbol_table.txt");
                    printf("Updated symbol table saved to output/symbol_table.txt\n");
                } else {
                    printf("\nNo new symbols loaded.\n");
                }
                press_enter_to_continue();
                break;
            }

            case 2: {
                char name[MAX_NAME_LEN], type[MAX_TYPE_LEN], scope[MAX_SCOPE_LEN];
                int size = 0, addr = -1;

                printf("\nEnter Symbol Name: ");
                fflush(stdout);
                if (!fgets(name, sizeof(name), stdin)) break;
                name[strcspn(name, "\r\n")] = '\0';

                printf("Enter Data Type (e.g., int, float, char, double): ");
                fflush(stdout);
                if (!fgets(type, sizeof(type), stdin)) break;
                type[strcspn(type, "\r\n")] = '\0';

                printf("Enter Scope [Default: global]: ");
                fflush(stdout);
                if (!fgets(scope, sizeof(scope), stdin)) break;
                scope[strcspn(scope, "\r\n")] = '\0';
                if (strlen(scope) == 0) strcpy(scope, "global");

                size = get_datatype_size(type);

                if (insert_symbol(&st, name, type, scope, addr, size)) {
                    printf("\nSymbol '%s' inserted successfully!\n", name);
                    save_symbol_table_to_file(&st, "output/symbol_table.txt");
                    printf("Updated symbol table saved to output/symbol_table.txt\n");
                }
                press_enter_to_continue();
                break;
            }

            case 3: {
                char name[MAX_NAME_LEN];
                printf("\nEnter Symbol Name to search: ");
                fflush(stdout);
                if (!fgets(name, sizeof(name), stdin)) break;
                name[strcspn(name, "\r\n")] = '\0';

                int idx = search_symbol(&st, name);
                if (idx != -1) {
                    const Symbol *s = &st.symbols[idx];
                    printf("\nSymbol Found:\n");
                    printf("----------------------------------------\n");
                    printf("Name      : %s\n", s->name);
                    printf("Data Type : %s\n", s->data_type);
                    printf("Scope     : %s\n", s->scope);
                    printf("Address   : %d\n", s->address);
                    printf("Size      : %d bytes\n", s->size);
                    printf("----------------------------------------\n");
                } else {
                    printf("\nSymbol '%s' not found in Symbol Table.\n", name);
                }
                press_enter_to_continue();
                break;
            }

            case 4: {
                char name[MAX_NAME_LEN];
                printf("\nEnter Symbol Name to delete: ");
                fflush(stdout);
                if (!fgets(name, sizeof(name), stdin)) break;
                name[strcspn(name, "\r\n")] = '\0';

                if (delete_symbol(&st, name)) {
                    printf("\nSymbol '%s' deleted successfully!\n", name);
                    save_symbol_table_to_file(&st, "output/symbol_table.txt");
                    printf("Updated symbol table saved to output/symbol_table.txt\n");
                } else {
                    printf("\nSymbol '%s' not found.\n", name);
                }
                press_enter_to_continue();
                break;
            }

            case 5: {
                char name[MAX_NAME_LEN], type[MAX_TYPE_LEN], scope[MAX_SCOPE_LEN];
                printf("\nEnter Symbol Name to update: ");
                fflush(stdout);
                if (!fgets(name, sizeof(name), stdin)) break;
                name[strcspn(name, "\r\n")] = '\0';

                int idx = search_symbol(&st, name);
                if (idx == -1) {
                    printf("\nSymbol '%s' not found.\n", name);
                    press_enter_to_continue();
                    break;
                }

                printf("Enter New Data Type (press Enter to keep '%s'): ", st.symbols[idx].data_type);
                fflush(stdout);
                if (!fgets(type, sizeof(type), stdin)) break;
                type[strcspn(type, "\r\n")] = '\0';

                printf("Enter New Scope (press Enter to keep '%s'): ", st.symbols[idx].scope);
                fflush(stdout);
                if (!fgets(scope, sizeof(scope), stdin)) break;
                scope[strcspn(scope, "\r\n")] = '\0';

                int new_size = (strlen(type) > 0) ? get_datatype_size(type) : st.symbols[idx].size;

                if (update_symbol(&st, name, type, scope, -1, new_size)) {
                    printf("\nSymbol '%s' updated successfully!\n", name);
                    save_symbol_table_to_file(&st, "output/symbol_table.txt");
                    printf("Updated symbol table saved to output/symbol_table.txt\n");
                }
                press_enter_to_continue();
                break;
            }

            case 6:
                display_symbol_table(&st);
                save_symbol_table_to_file(&st, "output/symbol_table.txt");
                printf("\nSymbol table saved to output/symbol_table.txt\n");
                press_enter_to_continue();
                break;

            default:
                printf("\nInvalid choice! Please select an option between 1 and 7.\n");
                press_enter_to_continue();
                break;
        }
    }
}
