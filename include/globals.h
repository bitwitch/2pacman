#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <stdint.h>
#include "structs.h"

#define SCREEN_WIDTH  896
#define SCREEN_HEIGHT 1152

#define BOARD_WIDTH  28
#define BOARD_HEIGHT 36

#define DOT_COUNT 244

#define SCALE 3
#define TILE_SIZE 8
#define TILE_RENDER_SIZE (TILE_SIZE * SCALE)

#define SEC_TO_USEC 1e6
#define USEC_TO_SEC 1e-6

#define TICRATE 60
#define TIME_STEP ((int64_t)SEC_TO_USEC / (int64_t)TICRATE)  /* microseconds per frame */


enum {
    BLINKY = 0,
    PINKY,
    INKY,
    CLYDE,
    GHOST_COUNT
};

extern game_t game;
extern ghost_t ghosts[GHOST_COUNT];
extern pacman_t pacman;
extern tilemap_t *tilemap;
extern char board[BOARD_WIDTH*BOARD_HEIGHT];


#endif
