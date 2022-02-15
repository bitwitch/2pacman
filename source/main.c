#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include "globals.h"
#include "ghosts.h"
#include "2pac.h"
#include "render.h"
#include "init.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
game_t game = {0};
char board[BOARD_WIDTH*BOARD_HEIGHT] = {0};
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
    game.timer.prev_time = game.timer.cur_time;
    game.timer.cur_time = get_time();
    game.timer.delta = (double)(game.timer.cur_time - game.timer.prev_time);
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
                case SDL_SCANCODE_RETURN:
                    game.enter = event.type == SDL_KEYDOWN;
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

static void set_ghostmode_timer(void) {
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

static void update_main_menu(void) {
    if (game.intro_timer < 12 * SEC_TO_USEC) {

    }

    game.intro_timer -= TIME_STEP;
    if (game.enter) {
        game.mode = GAME;
        game.intro_timer = 2 * SEC_TO_USEC;
    }
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
                frighten_ghosts(false);
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
        frighten_ghosts(true);
        game.prev_ghostmode = game.ghostmode;
        game.ghostmode = FLEE;
        printf("[INFO] Flee mode activated\n");
    }

    if (game.dots_remaining == 0) {
        /* TODO(shaw): level completed */
        printf("Level %d completed!\n", game.level);
        ++game.level;
        game.dots_remaining = DOT_COUNT;
        set_ghostmode_timer();
        game.intro_timer = 2 * SEC_TO_USEC;
        init_board(board);
        init_entities();
        frighten_ghosts(false);
        game.ghostmode = SCATTER;

    }
}

static void game_over(void) {
    
}

void restart_from_death(void) {
    set_ghostmode_timer();
    game.intro_timer = 2 * SEC_TO_USEC;
    init_entities();
    frighten_ghosts(false);
    game.ghostmode = SCATTER;
    pacman.dead = false;
}

static void check_ghost_collision(void) {
    if (pacman.dead) return;
    for (int i=0; i<GHOST_COUNT; ++i) {
        if (pacman.tile == ghosts[i].tile) {
            pacman.dead = true;
            pacman.death_timer = pacman.death_duration*SEC_TO_USEC;
            pacman.anim_timer = pacman.anim_frame_time;
            pacman.frame = 0;
            if (--pacman.lives < 0)
                game_over();
        }
    }
}

void update(void) {
    do_input();

    if (game.mode == MAIN_MENU) {
        update_main_menu();
    } else {
        if (game.intro_timer > 0) {
            game.intro_timer -= TIME_STEP;
        } else {
            update_ghostmode();
            update_ghosts();
            update_2pac();
            update_board();
            check_ghost_collision();
        }
    }
}

void init_game(void) {
    init_sdl();
    game.mode = MAIN_MENU;
    game.intro_timer = 12 * SEC_TO_USEC;
    game.ghostmode = SCATTER;
    game.level = 1;
    set_ghostmode_timer();
    game.dots_remaining = DOT_COUNT;
    /* the rest of the fields should have been zero initialized, game = {0}; */
}


int main(int argc, char **argv) {
    srand(0x94c3a2);

    init_game(); /* initializes global game object */

    spritesheet = load_texture("assets/spritesheet.png");

    init_board(board);
    init_tilemap();
    init_alphabet();
    init_entities();

    init_hud();
    init_menu_intro();

    set_scatter_targets();

    while(!game.quit) {
        tick();

        game.timer.accumulator += game.timer.delta;
        while (game.timer.accumulator > TIME_STEP) {
            update();
            game.timer.accumulator -= TIME_STEP;
        }

        float interp = game.timer.accumulator / TIME_STEP;

        if (game.mode == MAIN_MENU)
            render_menu(interp);
        else 
            render_game(board, BOARD_WIDTH*BOARD_HEIGHT, interp);
    }

    return 0;
}
