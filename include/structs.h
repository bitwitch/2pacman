#ifndef __STRUCTS_H
#define __STRUCTS_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "vecs.h"

typedef enum {
    SCATTER,
    CHASE,
    FLEE
} ghostmode_e;

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    bool up,down,left,right;
    ghostmode_e ghostmode, prev_ghostmode;
    int64_t ghostmode_timer;
    int64_t flee_timer;
    int level;
    int phase;
    bool quit;
    int dots_remaining;
    int score;
} game_t;


/* used for hashmap from characters to tiles */
typedef struct {
    char key;
    SDL_Rect value;
} tilemap_t;

typedef enum {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT
} dir_e;

typedef enum {
    HOUSE_PARTY,
    EXIT_HOUSE,
    NORMAL
} ghost_state_e;

typedef struct {
    v2f_t pos;
    dir_e dir;
    bool moving;
    bool reverse;
    int tile;
    float speed;
    char c;            /* character that maps to my spritesheet location */
    int next_tile;     /* the next tile i am heading towards */
    int target_tile;   /* the long term target tile i am heading towards */
    int scatter_target_tile;
    ghost_state_e state; 
    int64_t ghost_house_timer; 
} entity_t;

#endif
