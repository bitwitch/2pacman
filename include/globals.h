#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "structs.h"

#define SCREEN_WIDTH  896
#define SCREEN_HEIGHT 1152

#define BOARD_WIDTH  28
#define BOARD_HEIGHT 31

#define SCALE 3
#define TILE_SIZE 8
#define TILE_RENDER_SIZE (TILE_SIZE * SCALE)

#define SEC_TO_USEC 1e6

#define TICRATE 60
#define TIME_STEP (SEC_TO_USEC / TICRATE)  /* microseconds per frame */



enum {
    BLINKY = 0,
    PINKY,
    INKY,
    CLYDE,
    GHOST_COUNT
};

extern game_t game;
extern entity_t ghosts[GHOST_COUNT];
extern entity_t pacman;
extern tilemap_t *tilemap;
extern char board[BOARD_WIDTH*BOARD_HEIGHT];


#endif
