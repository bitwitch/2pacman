#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <SDL2/SDL.h>


#include "globals.h"
#include "ghosts.h"
#include "2pac.h"
#include "render.h"
#include "init.h"
#include "structs.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define TICRATE 144
#define TIME_STEP (1e6 / TICRATE) 

bool quit;
uint64_t cur_time, prev_time;
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
    prev_time = cur_time;
    cur_time = get_time();
    delta = (double)(cur_time - prev_time);
}


void update(void) {
    update_ghosts();
    update_2pac();
    SDL_Delay(16);
}



tilemap_t *tilemap = NULL;

void init_board(char *board) {
    assert(BOARD_WIDTH == 28 && BOARD_HEIGHT == 31);
    int offset = 0;
    memcpy(board+offset, "1^^^^^^^^^^^^^^^^^^^^^^^^^^2", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l............##............r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l.####.#####.##.#####.####.r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "lo####.#####.##.#####.####or", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l.####.#####.##.#####.####.r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l..........................r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l.####.##.########.##.####.r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l.####.##.########.##.####.r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l......##....##....##......r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "______.#####.##.#####.______", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     l.#####.##.#####.r     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     l.##..........##.r     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     l.##.###..###.##.r     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "^^^^^^.##.#.B..P.#.##.^^^^^^", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "..........#......#..........", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "______.##.#.C..I.#.##.______", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     l.##.########.##.r     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     l.##..........##.r     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     l.##.########.##.r     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "^^^^^^.##.########.##.^^^^^^", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l............##............r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l.####.#####.##.#####.####.r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l.####.#####.##.#####.####.r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "lo..##.......>........##..or", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l##.##.##.########.##.##.##r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l##.##.##.########.##.##.##r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l......##....##....##......r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l.##########.##.##########.r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l.##########.##.##########.r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "l..........................r", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "4__________________________3", BOARD_WIDTH); offset += BOARD_WIDTH;

    SDL_Rect none       = {0, 88, 8, 8};    hmput(tilemap, ' ', none);
    SDL_Rect top_wall   = {8, 0, 8, 8};     hmput(tilemap, '^', top_wall);
    SDL_Rect bot_wall   = {8, 72, 8, 8};    hmput(tilemap, '_', bot_wall);
    SDL_Rect left_wall  = {0, 8, 8, 8};     hmput(tilemap, 'l', left_wall);
    SDL_Rect right_wall = {216, 8, 8, 8};   hmput(tilemap, 'r', right_wall);
    SDL_Rect tl         = {0, 0, 8, 8};     hmput(tilemap, '1', tl);
    SDL_Rect tr         = {216, 0, 8, 8};   hmput(tilemap, '2', tr);
    SDL_Rect br         = {216, 280, 8, 8}; hmput(tilemap, '3', br);
    SDL_Rect bl         = {0, 280, 8, 8};   hmput(tilemap, '4', bl);


    SDL_Rect wall = {8, 0, 8, 8};     hmput(tilemap, '#', wall);

    /* TODO(shaw): change these */
    hmput(tilemap, '.', none);
    hmput(tilemap, 'o', none);
    hmput(tilemap, 'B', none);
    hmput(tilemap, 'P', none);
    hmput(tilemap, 'C', none);
    hmput(tilemap, 'I', none);
    hmput(tilemap, '>', none);
}

app_t app;
SDL_Texture *spritesheet;

int main(int argc, char **argv) {
    memset(&app, 0, sizeof(app_t));

    init_sdl();

    quit = false;
    cur_time = 0;
    prev_time = 0;
    delta = 0;
    accumulator = 0;

    char board[BOARD_WIDTH*BOARD_HEIGHT];
    init_board(board);

    spritesheet = load_texture("../assets/spritesheet.png");

    while(!quit) {
        tick();

        accumulator += delta;
        while (accumulator > TIME_STEP) {
            update();
            accumulator -= TIME_STEP;
        }

        float interp = accumulator / TIME_STEP;

        render(board, BOARD_WIDTH*BOARD_HEIGHT, interp);
    }

    return 0;
}
