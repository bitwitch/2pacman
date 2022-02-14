#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "vecs.h"

#define SCREEN_WIDTH  784
#define SCREEN_HEIGHT 1008

#define BOARD_WIDTH  28
#define BOARD_HEIGHT 36

#define DOT_COUNT 244

#define SCALE 3.5
#define TILE_SIZE 8
#define TILE_RENDER_SIZE (TILE_SIZE * SCALE)

#define SEC_TO_USEC 1e6
#define USEC_TO_SEC 1e-6

#define TICRATE 60
#define TIME_STEP ((int64_t)SEC_TO_USEC / (int64_t)TICRATE)  /* microseconds per frame */

#define MAX_HUD_ITEMS 12
#define MAX_INTRO_ITEMS 14

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

enum {
    BLINKY = 0,
    PINKY,
    INKY,
    CLYDE,
    GHOST_COUNT
};

typedef enum {
    MAIN_MENU,
    GAME
} mode_e;

typedef enum {
    SCATTER,
    CHASE,
    FLEE
} ghostmode_e;

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    bool up,down,left,right,enter;
    mode_e mode;
    ghostmode_e ghostmode, prev_ghostmode;
    int64_t ghostmode_timer;
    int64_t flee_timer;
    int64_t intro_timer;
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

/* used for hashmap from alpha characters to tiles */
typedef struct {
    char key;
    SDL_Rect value;
} alphabet_t;

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
    int w, h;
    int frame;
    bool moving;
    bool reverse;
    bool frightened;
    int tile;
    float speed;
    char c;            /* character that maps to my spritesheet location */
    int target_tile;   /* the long term target tile i am heading towards */
    int scatter_target_tile;
    ghost_state_e state; 
    int64_t ghost_house_timer; 
    int64_t anim_timer, anim_frame_time;
} ghost_t;

typedef struct {
    v2f_t pos;
    dir_e dir;
    int w, h;
    int frame;
    bool moving;
    int tile;
    float speed;
    char c;            /* character that maps to my spritesheet location */
    int target_tile;   /* the long term target tile i am heading towards */
    int64_t anim_timer, anim_frame_time;
} pacman_t;

typedef struct {
    bool show;
    float start_time;    /* in seconds */
    int size;
    v2f_t start_pos;
    SDL_Rect srcrects[BOARD_WIDTH];
    SDL_Rect dstrects[BOARD_WIDTH];
    int rect_count;
} menu_intro_item_t;

typedef struct {
    bool show;
    int size;
    v2f_t start_pos;
    SDL_Rect srcrects[BOARD_WIDTH];
    SDL_Rect dstrects[BOARD_WIDTH];
    int rect_count;
} sprite_row_t;


extern game_t game;
extern ghost_t ghosts[GHOST_COUNT];
extern pacman_t pacman;
extern tilemap_t *tilemap;
extern alphabet_t *alphabet;
extern char board[BOARD_WIDTH*BOARD_HEIGHT];
extern menu_intro_item_t menu_intro_items[MAX_INTRO_ITEMS];
extern sprite_row_t hud_items[MAX_HUD_ITEMS];

#endif
