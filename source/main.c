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
ghost_t ghosts[GHOST_COUNT] = {0};
pacman_t pacman;

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
    switch (game.ghostmode) {
        case SCATTER: 
            game.ghostmode_timer -= TIME_STEP;
            if (game.ghostmode_timer < 0) {
                game.ghostmode = CHASE;
                printf("[INFO] Chase mode activated\n");
                reverse_ghosts();
                set_ghostmode_timer();
            }
            break;

        case CHASE:   
            game.ghostmode_timer -= TIME_STEP;
            if (game.ghostmode_timer < 0 && game.phase < 7) {
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
                if (game.prev_ghostmode == SCATTER) {
                    set_scatter_targets();
                    game.ghostmode = SCATTER;
                    printf("[INFO] Scatter mode activated\n");
                } else {
                    game.ghostmode = CHASE;
                    printf("[INFO] Chase mode activated\n");
                }
            }
            break;

        default:
            assert(false && "unknown ghostmode in update_ghostmode");
            break;
    }
}

void update_board(void) {
    if (board[pacman.tile] == '.') {
        board[pacman.tile] = ' ';
        --game.dots_remaining;
        game.score += 10;
    }

    static int flee_times[18] = {6,5,4,3,2,5,2,2,1,5,2,1,1,3,1,1,0,1};
    if (board[pacman.tile] == '0') {
        board[pacman.tile] = ' ';
        --game.dots_remaining;
        game.score += 50;
        game.flee_timer = game.level > 18 ? 0 : flee_times[game.level-1]*SEC_TO_USEC;
        reverse_ghosts();
        game.prev_ghostmode = game.ghostmode;
        game.ghostmode = FLEE;
        printf("[INFO] Flee mode activated\n");
    }

    if (game.dots_remaining == 0) {
        /* TODO(shaw): level completed */
        printf("Level %d completed!\n", game.level);
    }
}

void update(void) {
    do_input();
    update_ghostmode();
    update_ghosts();
    update_2pac();
    update_board();
}


void init_entities() {
    float x,y;
    float speed = 0.814159;

    x = 111.0f; y = 116.0f;
    ghosts[BLINKY].c = 'B';
    ghosts[BLINKY].pos.x = x; 
    ghosts[BLINKY].pos.y = y;
    ghosts[BLINKY].w = 16;
    ghosts[BLINKY].h = 16;
    ghosts[BLINKY].tile = tile_at(ghosts[BLINKY].pos);
    ghosts[BLINKY].speed = speed;
    ghosts[BLINKY].dir = LEFT;
    ghosts[BLINKY].moving = true;
    ghosts[BLINKY].state = NORMAL;
    ghosts[BLINKY].scatter_target_tile = 25;
    ghosts[BLINKY].anim_frame_time = (int64_t)SEC_TO_USEC / 15;
    ghosts[BLINKY].anim_timer = ghosts[BLINKY].anim_frame_time;

    x = 95.0f; y = 140.0f;
    ghosts[INKY].c = 'I';
    ghosts[INKY].pos.x = x; 
    ghosts[INKY].pos.y = y;
    ghosts[INKY].w = 16;
    ghosts[INKY].h = 16;
    ghosts[INKY].tile = tile_at(ghosts[INKY].pos);
    ghosts[INKY].speed = speed;
    ghosts[INKY].dir = UP;
    ghosts[INKY].ghost_house_timer = 4 * SEC_TO_USEC;
    ghosts[INKY].moving = true;
    ghosts[INKY].state = HOUSE_PARTY;
    ghosts[INKY].scatter_target_tile = 36*BOARD_WIDTH;
    ghosts[INKY].anim_frame_time = (int64_t)SEC_TO_USEC / 15;
    ghosts[INKY].anim_timer = ghosts[INKY].anim_frame_time;

    x = 111.0f; y = 140.0f;
    ghosts[PINKY].c = 'P';
    ghosts[PINKY].pos.x = x; 
    ghosts[PINKY].pos.y = y;
    ghosts[PINKY].w = 16;
    ghosts[PINKY].h = 16;
    ghosts[PINKY].tile = tile_at(ghosts[PINKY].pos);
    ghosts[PINKY].speed = speed;
    ghosts[PINKY].dir = UP;
    ghosts[PINKY].state = HOUSE_PARTY;
    ghosts[PINKY].ghost_house_timer = 1 * SEC_TO_USEC;
    ghosts[PINKY].moving = true;
    ghosts[PINKY].scatter_target_tile = 2;
    ghosts[PINKY].anim_frame_time = (int64_t)SEC_TO_USEC / 15;
    ghosts[PINKY].anim_timer = ghosts[PINKY].anim_frame_time;

    x = 127.0f; y = 140.0f;
    ghosts[CLYDE].c = 'C';
    ghosts[CLYDE].pos.x = x; 
    ghosts[CLYDE].pos.y = y;
    ghosts[CLYDE].w = 16;
    ghosts[CLYDE].h = 16;
    ghosts[CLYDE].tile = tile_at(ghosts[CLYDE].pos);
    ghosts[CLYDE].speed = speed;
    ghosts[CLYDE].dir = UP;
    ghosts[CLYDE].state = HOUSE_PARTY;
    ghosts[CLYDE].ghost_house_timer = 7 * SEC_TO_USEC;
    ghosts[CLYDE].moving = true;
    ghosts[CLYDE].scatter_target_tile = 36*BOARD_WIDTH + 27;
    ghosts[CLYDE].anim_frame_time = (int64_t)SEC_TO_USEC / (int64_t)15;
    ghosts[CLYDE].anim_timer = ghosts[CLYDE].anim_frame_time;

    x = 111.0f; y = 212.0f;
    pacman.c = '>';
    pacman.pos.x = x;
    pacman.pos.y = y;
    pacman.w = 16;
    pacman.h = 16;
    pacman.tile = tile_at(pacman.pos);
    pacman.speed = speed;
    pacman.dir = LEFT;
    pacman.moving = true;
    pacman.anim_frame_time = (int64_t)SEC_TO_USEC / 15;
    pacman.anim_timer = pacman.anim_frame_time;
}

void init_game(void) {
    init_sdl();
    game.ghostmode = SCATTER;
    game.level = 1;
    set_ghostmode_timer();
    game.dots_remaining = DOT_COUNT;
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
