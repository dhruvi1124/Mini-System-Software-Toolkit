#ifndef MACRO_PROCESSOR_H
#define MACRO_PROCESSOR_H

#define MAX_MACROS 50
#define MAX_MDT_ENTRIES 200
#define MAX_PARAMS 10
#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 256
#define MAX_EXP_LINES 500

/* Macro Name Table (MNT) Entry */
typedef struct {
    int index;
    char name[MAX_NAME_LEN];
    int mdt_index; /* 1-based index into MDT */
    int mdt_end_index; /* 1-based index where macro definition ends */
    int param_count;
    char param_names[MAX_PARAMS][MAX_NAME_LEN];
} MNTEntry;

typedef struct {
    MNTEntry entries[MAX_MACROS];
    int count;
} MNT;

/* Macro Definition Table (MDT) Entry */
typedef struct {
    int index;
    char statement[MAX_LINE_LEN];
} MDTEntry;

typedef struct {
    MDTEntry entries[MAX_MDT_ENTRIES];
    int count;
} MDT;

/* Expanded Program Buffer */
typedef struct {
    char lines[MAX_EXP_LINES][MAX_LINE_LEN];
    int count;
} ExpandedProgram;

/* Public API */
void run_macro_processor(void);
void init_macro_processor_state(void);

int process_macros(const char *input_path);
void display_mnt(void);
void display_mdt(void);
void display_expanded_program(void);

int save_macro_output(const char *output_path);

#endif /* MACRO_PROCESSOR_H */
