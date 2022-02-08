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

#define TICRATE 60
#define TIME_STEP (1e6 / TICRATE) 

bool quit;
uint64_t cur_time, prev_time;
double delta, accumulator;

game_t game = {0};
char board[BOARD_WIDTH*BOARD_HEIGHT] = {0};
tilemap_t *tilemap = NULL;
SDL_Texture *spritesheet;
entity_t ghosts[GHOST_COUNT] = {0};
entity_t pacman;

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


void do_input(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (event.key.repeat == 0) {
                switch(event.key.keysym.scancode) {
                case SDL_SCANCODE_UP:
                    game.up = event.type == SDL_KEYDOWN;
                    break;
                case SDL_SCANCODE_DOWN:
                    game.down = event.type == SDL_KEYDOWN;
                    break;
                case SDL_SCANCODE_LEFT:
                    game.left = event.type == SDL_KEYDOWN;
                    break;
                case SDL_SCANCODE_RIGHT:
                    game.right = event.type == SDL_KEYDOWN;
                    break;
                default:
                    break;
                }

            }
            break;

        default:
            break;
        }
    }
}


void update(void) {
    do_input();
    update_ghosts();
    update_2pac();
}


void init_entities() {
    float x,y;

    x = 111.0f; y = 92.0f;
    ghosts[BLINKY].pos.x = x; 
    ghosts[BLINKY].pos.y = y;
    ghosts[BLINKY].tile = (int)y/8 * BOARD_WIDTH + (int)x/8;
    ghosts[BLINKY].speed = 3.14159f;
    ghosts[BLINKY].c = 'B';

    x = 95.0f; y = 116.0f;
    ghosts[INKY].pos.x = x; 
    ghosts[INKY].pos.y = y;
    ghosts[INKY].tile = (int)y/8 * BOARD_WIDTH + (int)x/8;
    ghosts[INKY].speed = 3.14159f;
    ghosts[INKY].c = 'I';

    x = 111.0f; y = 116.0f;
    ghosts[PINKY].pos.x = x; 
    ghosts[PINKY].pos.y = y;
    ghosts[PINKY].tile = (int)y/8 * BOARD_WIDTH + (int)x/8;
    ghosts[PINKY].speed = 3.14159f;
    ghosts[PINKY].c = 'P';

    x = 127.0f; y = 116.0f;
    ghosts[CLYDE].pos.x = x; 
    ghosts[CLYDE].pos.y = y;
    ghosts[CLYDE].tile = (int)y/8 * BOARD_WIDTH + (int)x/8;
    ghosts[CLYDE].speed = 3.14159f;
    ghosts[CLYDE].c = 'C';

    x = 111.0f; y = 188.0f;
    pacman.pos.x = x;
    pacman.pos.y = y;
    pacman.tile = (int)y/8 * BOARD_WIDTH + (int)x/8;
    pacman.speed = 0.814159f;
    pacman.dir = LEFT;
    pacman.moving = false;
    pacman.c = '>';
   
}

int main(int argc, char **argv) {
    init_sdl();

    init_board(board);
    init_entities();

    spritesheet = load_texture("../assets/spritesheet.png");

    quit = false;
    cur_time = 0;
    prev_time = 0;
    delta = 0;
    accumulator = 0;

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
