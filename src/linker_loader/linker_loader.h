#ifndef LINKER_LOADER_H
#define LINKER_LOADER_H

#define MAX_MODULES 10
#define MAX_EST_SYMBOLS 50
#define MAX_MEM_RECORDS 200
#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 256

/* External Symbol Table (EST) Entry */
typedef struct {
    char name[MAX_NAME_LEN];
    char module_name[MAX_NAME_LEN];
    int relative_address;
    int linked_address;
} ESTEntry;

typedef struct {
    ESTEntry entries[MAX_EST_SYMBOLS];
    int count;
} ExternalSymbolTable;

/* Individual Code/Data Record inside a module */
typedef struct {
    int address;                   /* Original address / offset */
    int value;                     /* Object code value */
    char symbol_ref[MAX_NAME_LEN]; /* Referenced external symbol name (or empty) */
} ModuleRecord;

/* Defined Symbol Entry inside a module */
typedef struct {
    char name[MAX_NAME_LEN];
    int address;
} DefSymbol;

/* Module Definition */
typedef struct {
    char name[MAX_NAME_LEN];
    int start_address;
    
    DefSymbol def_symbols[MAX_EST_SYMBOLS];
    int def_count;

    char ext_symbols[MAX_EST_SYMBOLS][MAX_NAME_LEN];
    int ext_count;

    ModuleRecord records[MAX_MEM_RECORDS];
    int record_count;
} Module;

/* Linked Memory Entry */
typedef struct {
    int address;                   /* Relocated linked address */
    int value;                     /* Object code value */
    char symbol_ref[MAX_NAME_LEN]; /* Symbol reference */
    int resolved_addr;             /* Resolved symbol address (-1 if none) */
    char module_name[MAX_NAME_LEN];
} LinkedMemoryEntry;

typedef struct {
    LinkedMemoryEntry entries[MAX_MEM_RECORDS];
    int count;
} LinkedMemoryMap;

typedef struct {
    Module modules[MAX_MODULES];
    int module_count;
    ExternalSymbolTable est;
    LinkedMemoryMap memory_map;
    int is_valid;
    char error_msg[MAX_LINE_LEN];
    int executed;
} LinkerResult;

/* Public API */
void run_linker_loader(void);

int link_and_load(const char *input_text, LinkerResult *result);
void display_est(const LinkerResult *result);
void display_memory_map(const LinkerResult *result);
int save_linker_output(const LinkerResult *result, const char *filepath);

#endif /* LINKER_LOADER_H */
