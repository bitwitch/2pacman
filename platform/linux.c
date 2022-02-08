#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <termios.h>

#include "globals.h"
#include "ghosts.h"
#include "2pac.h"
#include "render.h"

#define TICRATE 144
#define TIME_STEP (1e6 / TICRATE) /* microsecs per frame */

static bool quit;
uint64_t time, prev_time;
double delta, accumulator;

/*
 * idea taken from doom source
 * https://github.com/id-Software/DOOM
 */
uint64_t get_time(void)
{
    struct timeval tp;
    struct timezone tzp;
    float usecs;
    static int basetime=0;
    gettimeofday(&tp, &tzp);
    if (!basetime)
        basetime = tp.tv_sec;
    usecs = (tp.tv_sec-basetime)*1e6 + tp.tv_usec;
    return usecs;
}

void tick (void) {
    prev_time = time;
    time = get_time();
    delta = (double)(time - prev_time);
}


void update(void) {
    update_ghosts();
    update_2pac();
}


void init_board(void) {
    int offset = 0;
    memcpy(board+offset, "############################", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#............##............#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#.####.#####.##.#####.####.#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#o####.#####.##.#####.####o#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#.####.#####.##.#####.####.#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#..........................#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#.####.##.########.##.####.#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#.####.##.########.##.####.#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#......##....##....##......#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "######.#####.##.#####.######", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     #.#####.##.#####.#     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     #.##..........##.#     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     #.##.###..###.##.#     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "######.##.#.B..P.#.##.######", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "..........#......#..........", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "######.##.#.C..I.#.##.######", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     #.##.########.##.#     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     #.##..........##.#     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     #.##.########.##.#     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "######.##.########.##.######", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#............##............#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#.####.#####.##.#####.####.#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#.####.#####.##.#####.####.#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#o..##.......>........##..o#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "###.##.##.########.##.##.###", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "###.##.##.########.##.##.###", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#......##....##....##......#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#.##########.##.##########.#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#.##########.##.##########.#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "#..........................#", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "############################", BOARD_WIDTH); offset += BOARD_WIDTH;
}

void init_terminal(void) {
    printf("\x1b[2J"); /* clear screen */
    struct termios term_settings;
    tcgetattr(STDOUT_FILENO, &term_settings);
    term_settings.c_lflag &= ~ECHO;
    tcsetattr(STDOUT_FILENO, TCSANOW, &term_settings);
}

void sigint_handler(int signum) {
    struct termios term_settings;
    tcgetattr(STDOUT_FILENO, &term_settings);
    term_settings.c_lflag |= ECHO;
    tcsetattr(STDOUT_FILENO, TCSANOW, &term_settings);
    printf("\x1b[2J"); /* clear screen */
    _exit(130);
}

int main(int argc, char **argv) {
    time = 0;
    prev_time = 0;
    delta = 0;
    accumulator = 0;

    struct sigaction act = { .sa_handler = sigint_handler };
    sigaction(SIGINT, &act, NULL);

    init_board();
    init_terminal();

    while(!quit) {
        tick();

        accumulator += delta;
        while (accumulator > TIME_STEP) {
            update();
            accumulator -= TIME_STEP;
        }

        float interp = accumulator / TIME_STEP;
        render(interp);
    }

    return 0;
}
