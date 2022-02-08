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
    memset(board, 0, BOARD_WIDTH*BOARD_HEIGHT);
    int offset = 0;
    memcpy(board+offset, "1^^^^^^^^^^^^qw^^^^^^^^^^^^2", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{............[]............}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.u~~o.u~~~o.[].u~~~o.u~~o.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{0[  ].[   ].[].[   ].[  ]0}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.i--p.i---p.ip.i---p.i--p.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{..........................}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.u~~o.uo.u~~~~~~o.uo.u~~o.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.i--p.[].i--da--p.[].i--p.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{......[]....[]....[]......}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "4____6.[s~~o [] u~~f].5____3", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     {.[a--p ip i--d].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     {.[]    B     [].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     {.[] lgv  bgx [].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "^^^^^7.ip jI P C k ip.8^^^^^", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "      .   j      k   .      ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "_____6.uo j      k uo.5_____", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     {.[] zhhhhhhc [].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     {.[]          [].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     {.[] u~~~~~~o [].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "1^^^^7.ip i--da--p ip.8^^^^2", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{............[]............}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.u~~o.u~~~o.[].u~~~o.u~~o.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.i-d].i---p.ip.i---p.[a-p.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{0..[].......>........[]..0}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "e~o.[].uo.u~~~~~~o.uo.[].u~t", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "r-p.ip.[].i--da--p.[].ip.i-y", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{......[]....[]....[]......}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.u~~~~fs~~o.[].u~~fs~~~~o.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.i--------p.ip.i--------p.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{..........................}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "4__________________________3", BOARD_WIDTH); offset += BOARD_WIDTH;

    SDL_Rect none       = {0, 88, 8, 8};     hmput(tilemap, ' ', none);

    SDL_Rect top_wall   = {8, 0, 8, 8};      hmput(tilemap, '^', top_wall);
    SDL_Rect bot_wall   = {8, 72, 8, 8};     hmput(tilemap, '_', bot_wall);
    SDL_Rect left_wall  = {0, 8, 8, 8};      hmput(tilemap, '{', left_wall);
    SDL_Rect right_wall = {216, 8, 8, 8};    hmput(tilemap, '}', right_wall);
    SDL_Rect otl         = {0, 0, 8, 8};     hmput(tilemap, '1', otl); /* outer top left */
    SDL_Rect otr         = {216, 0, 8, 8};   hmput(tilemap, '2', otr);
    SDL_Rect obr         = {216, 240, 8, 8}; hmput(tilemap, '3', obr);
    SDL_Rect obl         = {0, 240, 8, 8};   hmput(tilemap, '4', obl);
    SDL_Rect itl         = {176, 72, 8, 8};  hmput(tilemap, '5', itl); /* inner top left */
    SDL_Rect itr         = {40, 72, 8, 8};   hmput(tilemap, '6', itr);
    SDL_Rect ibr         = {40, 104, 8, 8};  hmput(tilemap, '7', ibr);
    SDL_Rect ibl         = {176, 104, 8, 8}; hmput(tilemap, '8', ibl);

    SDL_Rect t1         = {104, 0, 8, 8};  hmput(tilemap, 'q', t1);
    SDL_Rect t2         = {112, 0, 8, 8};  hmput(tilemap, 'w', t2);
    SDL_Rect t3         = {0, 192, 8, 8};  hmput(tilemap, 'e', t3);
    SDL_Rect t4         = {0, 200, 8, 8};  hmput(tilemap, 'r', t4);
    SDL_Rect t5         = {216, 192, 8, 8};  hmput(tilemap, 't', t5);
    SDL_Rect t6         = {216, 200, 8, 8};  hmput(tilemap, 'y', t6);

    SDL_Rect bt         = {24, 16, 8, 8};  hmput(tilemap, '~', bt); /* box top */
    SDL_Rect bb         = {24, 32, 8, 8};  hmput(tilemap, '-', bb);
    SDL_Rect bl         = {16, 24, 8, 8};  hmput(tilemap, '[', bl); /* box top */
    SDL_Rect br         = {40, 24, 8, 8};  hmput(tilemap, ']', br);
    SDL_Rect botl        = {16, 16, 8, 8}; hmput(tilemap, 'u', botl); /* box outer top left*/
    SDL_Rect bobl        = {16, 32, 8, 8}; hmput(tilemap, 'i', bobl);
    SDL_Rect botr        = {40, 16, 8, 8}; hmput(tilemap, 'o', botr); 
    SDL_Rect bobr        = {40, 32, 8, 8}; hmput(tilemap, 'p', bobr);
    SDL_Rect bitl        = {64, 80, 8, 8};  hmput(tilemap, 'a', bitl); /* box inner top left*/
    SDL_Rect bibl        = {64, 72, 8, 8};  hmput(tilemap, 's', bibl);
    SDL_Rect bitr        = {152, 80, 8, 8}; hmput(tilemap, 'd', bitr); 
    SDL_Rect bibr        = {152, 72, 8, 8}; hmput(tilemap, 'f', bibr);

    SDL_Rect ct  = {88, 96, 8, 8};   hmput(tilemap, 'g', ct); /* cage top*/
    SDL_Rect cb  = {88, 128, 8, 8};  hmput(tilemap, 'h', cb);
    SDL_Rect cl  = {80, 104, 8, 8};  hmput(tilemap, 'j', cl); 
    SDL_Rect cr  = {136, 104, 8, 8}; hmput(tilemap, 'k', cr);
    SDL_Rect ctl = {80, 96, 8, 8};   hmput(tilemap, 'l', ctl); /* cage top left*/
    SDL_Rect cbl = {80, 128, 8, 8};  hmput(tilemap, 'z', cbl);
    SDL_Rect ctr = {136, 96, 8, 8};  hmput(tilemap, 'x', ctr); 
    SDL_Rect cbr = {136, 128, 8, 8}; hmput(tilemap, 'c', cbr);
    SDL_Rect cle = {96, 96, 8, 8};  hmput(tilemap, 'v', cle); 
    SDL_Rect cre = {120, 96, 8, 8}; hmput(tilemap, 'b', cre);


    /* TODO(shaw): change these */
    SDL_Rect dot    = {388, 56, 8, 8};  hmput(tilemap, '.', dot);
    SDL_Rect pellet = {388, 48, 8, 8};  hmput(tilemap, '0', pellet);
    SDL_Rect blinky = {228, 64, 16, 16};  hmput(tilemap, 'B', blinky);
    SDL_Rect pinky  = {228, 80, 16, 16};  hmput(tilemap, 'P', pinky);
    SDL_Rect inky   = {228, 96, 16, 16};  hmput(tilemap, 'I', inky);
    SDL_Rect clyde  = {228, 112, 16, 16}; hmput(tilemap, 'C', clyde);
    SDL_Rect pacman = {244, 16, 16, 16};  hmput(tilemap, '>', pacman);


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
