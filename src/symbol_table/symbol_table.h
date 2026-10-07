#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_SYMBOLS 200
#define MAX_NAME_LEN 50
#define MAX_TYPE_LEN 20
#define MAX_SCOPE_LEN 20

typedef struct {
    char name[MAX_NAME_LEN];
    char data_type[MAX_TYPE_LEN];
    char scope[MAX_SCOPE_LEN];
    int address;
    int size;
} Symbol;

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    int count;
    int next_address;
} SymbolTable;

/* Public API */
void run_symbol_table(void);
void init_symbol_table(SymbolTable *st);
int insert_symbol(SymbolTable *st, const char *name, const char *data_type, const char *scope, int address, int size);
int search_symbol(const SymbolTable *st, const char *name);
int delete_symbol(SymbolTable *st, const char *name);
int update_symbol(SymbolTable *st, const char *name, const char *new_type, const char *new_scope, int new_address, int new_size);
void display_symbol_table(const SymbolTable *st);
int save_symbol_table_to_file(const SymbolTable *st, const char *filepath);
int load_symbols_from_file(SymbolTable *st, const char *filepath);
int get_datatype_size(const char *datatype);

#endif /* SYMBOL_TABLE_H */
