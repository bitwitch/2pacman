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

void update_ghostmode() {
    switch (game.ghostmode) {
        case SCATTER: 
            if (game.ghostmode_timer < 0) {
                printf("CHASE MODE ACTIVATED\n");
                /* set chase targets ???? */
                game.ghostmode = CHASE;
                game.ghostmode_timer = 20 * SEC_TO_USEC;
            }
            break;

        case CHASE:   
            if (game.ghostmode_timer < 0) {
                printf("SCATTER MODE ACTIVATED\n");
                set_scatter_targets();
                game.ghostmode = SCATTER;
                game.ghostmode_timer = 7 * SEC_TO_USEC;
            }
            break;

        case FLEE:
            if (game.ghostmode_timer < 0) {
                printf("SCATTER MODE ACTIVATED\n");
                set_scatter_targets();
                game.ghostmode = SCATTER;
                game.ghostmode_timer = 7 * SEC_TO_USEC;
            }
            break;

        default:
            assert(false && "unknown ghostmode in update_ghostmode");
            break;
    }
}

void update(void) {
    do_input();
    update_ghostmode();
    update_ghosts();
    update_2pac();
}


void init_entities() {
    float x,y;
    float speed = 0.814159;

    x = 111.0f; y = 92.0f;
    ghosts[BLINKY].c = 'B';
    ghosts[BLINKY].pos.x = x; 
    ghosts[BLINKY].pos.y = y;
    ghosts[BLINKY].tile = (int)y/8 * BOARD_WIDTH + (int)x/8;
    ghosts[BLINKY].speed = speed;
    ghosts[BLINKY].dir = LEFT;
    ghosts[BLINKY].moving = true;
    ghosts[BLINKY].state = NORMAL;
    ghosts[BLINKY].scatter_target_tile = 27;

    x = 95.0f; y = 116.0f;
    ghosts[INKY].c = 'I';
    ghosts[INKY].pos.x = x; 
    ghosts[INKY].pos.y = y;
    ghosts[INKY].tile = (int)y/8 * BOARD_WIDTH + (int)x/8;
    ghosts[INKY].speed = speed;
    ghosts[INKY].dir = UP;
    ghosts[INKY].ghost_house_timer = 4 * SEC_TO_USEC;
    ghosts[INKY].moving = true;
    ghosts[INKY].state = HOUSE_PARTY;
    ghosts[INKY].scatter_target_tile = 36*BOARD_WIDTH;

    x = 111.0f; y = 116.0f;
    ghosts[PINKY].c = 'P';
    ghosts[PINKY].pos.x = x; 
    ghosts[PINKY].pos.y = y;
    ghosts[PINKY].tile = (int)y/8 * BOARD_WIDTH + (int)x/8;
    ghosts[PINKY].speed = speed;
    ghosts[PINKY].dir = UP;
    ghosts[PINKY].ghost_house_timer = 1 * SEC_TO_USEC;
    ghosts[PINKY].moving = true;
    ghosts[PINKY].state = HOUSE_PARTY;
    ghosts[PINKY].scatter_target_tile = 0;

    x = 127.0f; y = 116.0f;
    ghosts[CLYDE].c = 'C';
    ghosts[CLYDE].pos.x = x; 
    ghosts[CLYDE].pos.y = y;
    ghosts[CLYDE].tile = (int)y/8 * BOARD_WIDTH + (int)x/8;
    ghosts[CLYDE].speed = speed;
    ghosts[CLYDE].dir = UP;
    ghosts[CLYDE].ghost_house_timer = 7 * SEC_TO_USEC;
    ghosts[CLYDE].moving = true;
    ghosts[CLYDE].state = HOUSE_PARTY;
    ghosts[CLYDE].scatter_target_tile = 36*BOARD_WIDTH + 27;

    x = 111.0f; y = 188.0f;
    pacman.c = '>';
    pacman.pos.x = x;
    pacman.pos.y = y;
    pacman.tile = (int)y/8 * BOARD_WIDTH + (int)x/8;
    pacman.speed = speed;
    pacman.dir = LEFT;
    pacman.moving = false;
}

int main(int argc, char **argv) {
    init_sdl();

    init_board(board);
    init_entities();

    spritesheet = load_texture("../assets/spritesheet.png");

    game.ghostmode = SCATTER;
    game.ghostmode_timer = 20 * SEC_TO_USEC;
    set_scatter_targets();

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
