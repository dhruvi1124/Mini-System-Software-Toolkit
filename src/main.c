#include <stdio.h>
#include <stdlib.h>

#include "common/common.h"
#include "lexical_analyzer/lexical_analyzer.h"
#include "symbol_table/symbol_table.h"
#include "two_pass_assembler/assembler.h"
#include "macro_processor/macro_processor.h"
#include "linker_loader/linker_loader.h"
#include "recursive_descent_parser/parser.h"
#include "quadruple_generator/quadruple.h"
#include "code_optimizer/optimizer.h"

static void display_main_menu(void)
{
    printf("\n========================================\n");
    printf("     MINI SYSTEM SOFTWARE TOOLKIT\n");
    printf("========================================\n\n");
    printf("1. Lexical Analyzer\n");
    printf("2. Symbol Table\n");
    printf("3. Two Pass Assembler\n");
    printf("4. Macro Processor\n");
    printf("5. Recursive Descent Parser\n");
    printf("6. Quadruple Generator\n");
    printf("7. Linker Loader\n");
    printf("8. Code Optimizer\n");
    printf("9. Exit\n\n");
    printf("Enter your choice: ");
    fflush(stdout);
}

int main(void)
{
    int choice = 0;
    int running = 1;

    while (running)
    {
        display_main_menu();

        if (scanf("%d", &choice) != 1)
        {
            printf("\nInvalid input! Please enter a number between 1 and 9.\n");
            clear_input_buffer();
            press_enter_to_continue();
            continue;
        }

        clear_input_buffer();

        switch (choice)
        {
        case 1:
            run_lexical_analyzer();
            break;
        case 2:
            run_symbol_table();
            break;
        case 3:
            run_two_pass_assembler();
            break;
        case 4:
            run_macro_processor();
            break;
        case 5:
            run_recursive_descent_parser();
            break;
        case 6:
            run_quadruple_generator();
            break;
        case 7:
            run_linker_loader();
            break;
        case 8:
            run_code_optimizer();
            break;
        case 9:
            printf("\nExiting Mini System Software Toolkit. Goodbye!\n");
            running = 0;
            break;
        default:
            printf("\nInvalid choice! Please select an option between 1 and 9.\n");
            press_enter_to_continue();
            break;
        }
    }
    return 0;
}
