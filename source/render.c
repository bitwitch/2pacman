#include <SDL2/SDL.h>
#include "globals.h"
#include "render.h"
#include "stb_image.h"
#include "stb_ds.h"

extern SDL_Texture *spritesheet;
extern tilemap_t *tilemap;

void render(char *board, int board_size, float interp) {
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app.renderer);

    int i, x, y;
    for (i=0; i<board_size; ++i) {
        x = i % BOARD_WIDTH;
        y = i / BOARD_WIDTH;
        SDL_Rect srcrect = hmget(tilemap, board[i]);
        int scale = 8*3;
        SDL_Rect dstrect = {.x = x*scale+15, .y = y*scale+15, .w = scale, .h = scale};
        SDL_RenderCopy(app.renderer, spritesheet, &srcrect, &dstrect);
    }


    SDL_RenderPresent(app.renderer);
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

    texture = SDL_CreateTextureFromSurface(app.renderer, surface);

    SDL_FreeSurface(surface);
    stbi_image_free(texture_pixels);

    return texture;
}
