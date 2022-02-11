#include <SDL2/SDL.h>
#include "globals.h"
#include "render.h"
#include "stb_image.h"
#include "stb_ds.h"

extern SDL_Texture *spritesheet;

void render(char *board, int board_size, float interp) {
    SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
    SDL_RenderClear(game.renderer);

    /* render board */
    int i, x, y;
    for (i=0; i<board_size; ++i) {
        x = i % BOARD_WIDTH;
        y = i / BOARD_WIDTH;
        SDL_Rect srcrect = hmget(tilemap, board[i]);
        SDL_Rect dstrect = {.x = x*TILE_RENDER_SIZE+15, .y = y*TILE_RENDER_SIZE+15, .w = TILE_RENDER_SIZE, .h = TILE_RENDER_SIZE};
        SDL_RenderCopy(game.renderer, spritesheet, &srcrect, &dstrect);
        if (i == pacman.tile) {
            SDL_SetRenderDrawColor(game.renderer, 0, 255, 0, 255);
            SDL_RenderDrawRect(game.renderer, &dstrect);
        }
    }

    /* render entities */
    for (int i=0; i<GHOST_COUNT; ++i) {
        entity_t ghost = ghosts[i];
        SDL_Rect srcrect = hmget(tilemap, ghost.c);
        x = (int)(ghost.pos.x + 0.5) - TILE_SIZE;
        y = (int)(ghost.pos.y + 0.5) - TILE_SIZE;
        SDL_Rect dstrect = {.x = x*SCALE+15, .y = y*SCALE+15, .w = 2*TILE_RENDER_SIZE, .h = 2*TILE_RENDER_SIZE};
        SDL_RenderCopy(game.renderer, spritesheet, &srcrect, &dstrect);

        /* TEMPORARILY DRAW THE TARGET TILES */
        switch (i) {
        case BLINKY: SDL_SetRenderDrawColor(game.renderer, 255, 0, 0, 255); break;
        case PINKY: SDL_SetRenderDrawColor(game.renderer, 229, 126, 252, 255); break;
        case INKY: SDL_SetRenderDrawColor(game.renderer, 200, 242, 252, 255); break;
        case CLYDE: SDL_SetRenderDrawColor(game.renderer, 252, 165, 65, 255); break;
        }
        v2f_t tile_pos = get_tile_pos(ghost.target_tile);
        SDL_Rect tile_rect = {
            .x = (tile_pos.x-0.5*TILE_SIZE)*SCALE+15, 
            .y = (tile_pos.y-0.5*TILE_SIZE)*SCALE+15, 
            .w = TILE_RENDER_SIZE, 
            .h = TILE_RENDER_SIZE
        };
        SDL_RenderDrawRect(game.renderer, &tile_rect);
    }

    SDL_Rect srcrect = hmget(tilemap, pacman.c);
    x = (int)(pacman.pos.x + 0.5) - TILE_SIZE;
    y = (int)(pacman.pos.y + 0.5) - TILE_SIZE;
    SDL_Rect dstrect = {.x = x*SCALE+15, .y = y*SCALE+15, .w = 2*TILE_RENDER_SIZE, .h = 2*TILE_RENDER_SIZE};
    SDL_RenderCopy(game.renderer, spritesheet, &srcrect, &dstrect);

    SDL_RenderPresent(game.renderer);
}

SDL_Texture *load_texture(char *filename) {
    SDL_Surface *surface;
    SDL_Texture *texture;
    int texture_width, texture_height;
    unsigned char *texture_pixels;
    int color_format = STBI_rgb_alpha;

    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

    texture_pixels = stbi_load(filename, &texture_width, &texture_height, NULL, color_format);

    if(texture_pixels == NULL) {
        /*TODO(shaw): try to load a default texture if this one is not found*/
        SDL_Log("[ERROR] Loading image %s failed: %s", filename, stbi_failure_reason());
        return NULL;
    }

    // Set up the pixel format color masks for RGB(A) byte arrays.
    Uint32 r_mask, g_mask, b_mask, a_mask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = (color_format == STBI_rgb) ? 8 : 0;
    r_mask = 0xff000000 >> shift;
    g_mask = 0x00ff0000 >> shift;
    b_mask = 0x0000ff00 >> shift;
    a_mask = 0x000000ff >> shift;
#else // little endian, like x86
    r_mask = 0x000000ff;
    g_mask = 0x0000ff00;
    b_mask = 0x00ff0000;
    a_mask = (color_format == STBI_rgb) ? 0 : 0xff000000;
#endif

    int depth, pitch;
    if (color_format == STBI_rgb) {
        depth = 24;
        pitch = 3*texture_width; // 3 bytes per pixel * pixels per row
    } else { // STBI_rgb_alpha (RGBA)
        depth = 32;
        pitch = 4*texture_width;
    }

    surface = SDL_CreateRGBSurfaceFrom(
        texture_pixels,
        texture_width,
        texture_height,
        depth,
        pitch,
        r_mask,
        g_mask,
        b_mask,
        a_mask);

    if (surface == NULL) {
      SDL_Log("Creating surface failed: %s", SDL_GetError());
      stbi_image_free(texture_pixels);
      return NULL;
    }

    texture = SDL_CreateTextureFromSurface(game.renderer, surface);

    SDL_FreeSurface(surface);
    stbi_image_free(texture_pixels);

    return texture;
}
