#ifndef __STRUCTS_H
#define __STRUCTS_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "vecs.h"

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    bool up,down,left,right;
} game_t;

/* used for hashmap from characters to tiles */
typedef struct {
    char key;
    SDL_Rect value;
} tilemap_t;


typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} dir_e;

typedef struct {
    v2f_t pos;
    dir_e dir;
    bool moving;
    int w, h;
    int tile;
    float speed;
    char c;       /* character that maps to spritesheet location */
} entity_t;

#endif
