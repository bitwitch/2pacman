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

/* TODO(shaw): persist the high score */
int high_score = 0;

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

static void update_hud(void) {
    assert(game.score <= 999999);
    int i = hud_items[SCORE].rect_count - 1;
    int n = game.score;
    char c, digit;
    while (n > 0) {
        digit = (char)(n % 10);
        c = digit + 48;
        hud_items[SCORE].srcrects[i--] = hmget(alphabet, c);
        n /= 10;
    }

    if (high_score == game.score) {
        memcpy(hud_items[HIGH_SCORE].srcrects, 
               hud_items[SCORE].srcrects, 
               sizeof(hud_items[SCORE].srcrects));
    }
}


static void update_main_menu(void) {
    game.intro_timer -= TIME_STEP;
    if (game.enter) {
        game.mode = GET_READY;
        game.intro_timer = game.get_ready_duration;
        hud_items[READY].show = true;
        hud_items[LIFE1].show = true;
        hud_items[LIFE2].show = true;
        hud_items[LIFE3].show = true;
        hud_items[CREDITS].show = false;
    }
}


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
            if (game.ghost_eaten_timer <= 0)
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
                game.ghosts_eaten = 0;
            }
            break;

        default:
            assert(false && "unknown ghostmode in update_ghostmode");
            break;
    }
}

static void score_points(int points) {
    game.score += points;
    if (game.score > high_score)
        high_score = game.score;
    update_hud();
}

void update_board(void) {
    static int flee_times[18] = {6,5,4,3,2,5,2,2,1,5,2,1,1,3,1,1,0,1};

    if (board[pacman.tile] == '.') {
        pacman.delay_frames = 1;
        board[pacman.tile] = ' ';
        --game.dots_remaining;
        score_points(10);
    }

    else if (board[pacman.tile] == '0') {
        pacman.delay_frames = 3;
        board[pacman.tile] = ' ';
        --game.dots_remaining;
        score_points(50);
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
        init_board();
        init_ghosts();
        reset_pacman();
        frighten_ghosts(false);
        game.ghostmode = SCATTER;
        set_scatter_targets();
    }
}

static void show_eat_points(int ghosts_eaten, v2f_t pos) {
    game.eat_points_sprite.show = true;
    game.eat_points_sprite.pos = pos;
    game.eat_points_sprite.srcrect.x = 372 + ghosts_eaten * game.eat_points_sprite.w;
}

void restart_from_death(void) {
    set_ghostmode_timer();
    game.intro_timer = game.get_ready_duration;
    game.mode = GET_READY;
    hud_items[READY].show = true;
    init_ghosts();
    reset_pacman();
    frighten_ghosts(false);
    game.ghostmode = SCATTER;
    pacman.dead = false;
}

static void check_ghost_collision(void) {
    if (pacman.dead) return;
    for (int i=0; i<GHOST_COUNT; ++i) {
        if (ghosts[i].state == GO_HOME) 
            continue;
        if (tile_at(pacman.pos) == ghosts[i].tile) {
            if (ghosts[i].frightened) {
                ghosts[i].target_tile = ghosts[i].respawn_tile;
                ghosts[i].state = GO_HOME;
                ghosts[i].frightened = false;
                ghosts[i].show = false;
                game.ghost_eaten_timer = 1*SEC_TO_USEC;
                score_points(200 * pow(2, game.ghosts_eaten));   /* 200, 400, 800, 1600 */ 
                show_eat_points(game.ghosts_eaten, ghosts[i].pos);
                ++game.ghosts_eaten;
            } else {
#ifndef INVINCIBLE
                pacman.dead = true;
                pacman.death_timer = pacman.death_duration*SEC_TO_USEC;
                pacman.anim_timer = pacman.anim_frame_time;
                pacman.frame = 0;
                --pacman.lives;
                break;
#endif
            }
        }
    }
}

static void update_get_ready(void) {
    if (game.intro_timer == game.get_ready_duration) {
        printf("lives: %d\n", pacman.lives);
        if (pacman.lives == 3)
            hud_items[LIFE3].show = false;
        else if (pacman.lives == 2)
            hud_items[LIFE2].show = false;
        else if (pacman.lives == 1)
            hud_items[LIFE1].show = false;
    }

    if (game.intro_timer > 0) {
        game.intro_timer -= TIME_STEP;
    } else {
        hud_items[READY].show = false;
        game.mode = GAME;
    }
}

static void reset_game(void) {
    game.mode = MAIN_MENU;
    game.intro_timer = 12 * SEC_TO_USEC;
    game.blink_timer = game.blink_interval;
    game.ghostmode = SCATTER;
    set_scatter_targets();
    game.level = 1;
    game.score = 0;
    set_ghostmode_timer();
    game.dots_remaining = DOT_COUNT;
}

static void update_game_over(void) {
    game.intro_timer -= TIME_STEP;
    if (game.intro_timer < 0) {
        hud_items[GAME_OVER_LABEL].show = false;
        hud_items[CREDITS].show = true;
        init_board();
        init_ghosts();
        init_pacman();
        reset_game();
    }
}

static void update_blink_timer(void) {
    game.blink_timer -= TIME_STEP;
    if (game.blink_timer <= 0)
        game.blink_timer = game.blink_interval;
}

static void update_eat_points(void) {
    if (game.ghost_eaten_timer > 0)
        game.ghost_eaten_timer -= TIME_STEP;
    else if (game.eat_points_sprite.show)
        game.eat_points_sprite.show = false;
}

void update(void) {
    do_input();

    switch (game.mode) {
    case MAIN_MENU: 
        update_main_menu(); 
        break;
    case GET_READY: 
        update_get_ready(); 
        break;
    case GAME: 
        update_eat_points();
        update_blink_timer();
        update_ghostmode();
        update_ghosts();
        update_2pac();
        check_ghost_collision();
        update_board();
        break;
    case GAME_OVER: 
        update_game_over();
        break;
    default:
        assert(false && "unknown game mode in update");
        break;
    }
}

void init_game(void) {
    init_sdl();
    game.mode = MAIN_MENU;
    game.intro_timer = 12 * SEC_TO_USEC;
    game.blink_interval = 0.25*SEC_TO_USEC;
    game.blink_timer = game.blink_interval;
    game.get_ready_duration = 2*SEC_TO_USEC;
    game.game_over_duration = 4*SEC_TO_USEC;
    game.ghostmode = SCATTER;
    game.level = 1;
    game.full_speed = 1.26262626;
    set_ghostmode_timer();
    game.dots_remaining = DOT_COUNT;
    game.eat_points_sprite.w = 16;
    game.eat_points_sprite.h = 16;
    game.eat_points_sprite.srcrect.y = 16;
    game.eat_points_sprite.srcrect.w = 16;
    game.eat_points_sprite.srcrect.h = 16;
    /* the rest of the fields should have been zero initialized, game = {0}; */
}


int main(int argc, char **argv) {
    srand(0x94c3a2);

    init_game(); /* initializes global game object */

    spritesheet = load_texture("assets/spritesheet.png");

    init_board();
    init_tilemap();
    init_alphabet();
    init_ghosts();
    init_pacman();

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
