#ifndef __STRUCTS_H
#define __STRUCTS_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
}app_t;

/* used for hashmap from characters to tiles */
typedef struct {
    char key;
    SDL_Rect value;
} tilemap_t;


#endif
