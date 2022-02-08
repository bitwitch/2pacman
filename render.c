#include <stdio.h>
#include <unistd.h>
#include "globals.h"
#include "render.h"


char board[BOARD_WIDTH*BOARD_HEIGHT];

void render(float interp) {
    /* rewind cursor */
    printf("\x1b[H"); 

    for (int i=0; i<BOARD_WIDTH*BOARD_HEIGHT; ++i) {
       if (i % BOARD_WIDTH == 0)
           fputc('\n', stdout);
       fputc(board[i], stdout);
    }

    sleep(0.005);
}
