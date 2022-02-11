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
            game.quit = true;
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


void set_ghostmode_timer() {
    static float phases[3][8] = {
       {7,20,7,20,5,20,5,-1},
       {7,20,7,20,5,1033,1/60,-1},
       {5,20,5,20,5,1037,1/60,-1},
    };

    if (game.phase > 8) {
        printf("[WARNING] Game phase %d is greater than 7 and set_ghostmode_timer was called. There are only 8 phases.\n", game.phase);
        return;
    }

    if (game.level == 1)
        game.ghostmode_timer = phases[0][game.phase] * SEC_TO_USEC;
    else if (game.level < 5)
        game.ghostmode_timer = phases[1][game.phase] * SEC_TO_USEC;
    else 
        game.ghostmode_timer = phases[2][game.phase] * SEC_TO_USEC;
}

/*
   Level 1
   Phase1. scatter for 7s
   Phase2. chase for 20s
   Phase3. scatter for 7s
   Phase4. chase for 20s
   Phase5. scatter for 5s
   Phase6. chase for 20s
   Phase7. scatter for 5s
   Phase8. chase forever

   Levels 2-4
   Phase1. scatter for 7s
   Phase2. chase for 20s
   Phase3. scatter for 7s
   Phase4. chase for 20s
   Phase5. scatter for 5s
   Phase6. chase for 1033s
   Phase7. scatter for .0166s
   Phase8. chase forever

   Levels 5+
   Phase1. scatter for 5s
   Phase2. chase for 20s
   Phase2. scatter for 5s
   Phase2. chase for 20s
   Phase3. scatter for 5s
   Phase2. chase for 1037s
   Phase7. scatter for .0166s
   Phase8. chase forever
*/
void update_ghostmode() {
    game.phase_last_tic = game.phase;
    switch (game.ghostmode) {
        case SCATTER: 
            game.ghostmode_timer -= TIME_STEP;
            if (game.ghostmode_timer < 0) {
                game.prev_phase = game.phase++;
                game.ghostmode = CHASE;
                printf("[INFO] Chase mode activated\n");
                reverse_ghosts();
                set_ghostmode_timer();
            }
            break;

        case CHASE:   
            game.ghostmode_timer -= TIME_STEP;
            if (game.ghostmode_timer < 0 && game.phase < 7) {
                game.prev_phase = game.phase++;
                game.ghostmode = SCATTER;
                set_scatter_targets();
                printf("[INFO] Scatter mode activated\n");
                reverse_ghosts();
                set_ghostmode_timer();
            }
            break;

        case FLEE:
            game.flee_timer -= TIME_STEP;
            if (game.flee_timer < 0) {
                printf("[INFO] Flee mode completed\n");
                /* TODO(shaw) need to actually go back to whatever state we were in before flee was triggered */
                set_scatter_targets();
                game.ghostmode = SCATTER;
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
    ghosts[BLINKY].tile = tile_at(ghosts[BLINKY].pos);
    ghosts[BLINKY].speed = speed;
    ghosts[BLINKY].dir = LEFT;
    ghosts[BLINKY].moving = true;
    ghosts[BLINKY].state = NORMAL;
    ghosts[BLINKY].scatter_target_tile = 27;

    x = 95.0f; y = 116.0f;
    ghosts[INKY].c = 'I';
    ghosts[INKY].pos.x = x; 
    ghosts[INKY].pos.y = y;
    ghosts[INKY].tile = tile_at(ghosts[INKY].pos);
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
    ghosts[PINKY].tile = tile_at(ghosts[PINKY].pos);
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
    ghosts[CLYDE].tile = tile_at(ghosts[CLYDE].pos);
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
    pacman.tile = tile_at(pacman.pos);
    pacman.speed = speed;
    pacman.dir = LEFT;
    pacman.moving = false;
}

void init_game(void) {
    init_sdl();
    game.ghostmode = SCATTER;
    game.level = 1;
    set_ghostmode_timer();
    /* the rest of the fields should have been zero initialized, game = {0}; */
}

int main(int argc, char **argv) {
    init_game(); /* initializes global game object */

    init_board(board);
    init_entities();

    spritesheet = load_texture("assets/spritesheet.png");

    set_scatter_targets();

    cur_time = 0;
    prev_time = 0;
    delta = 0;
    accumulator = 0;

    while(!game.quit) {
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
