#include <stdio.h>
#include "common.h"

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* Consume extra characters from input buffer */
    }
}

void press_enter_to_continue(void) {
    printf("Press Enter to return to Main Menu...");
    fflush(stdout);
    getchar();
}
