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
#define BONUS_COUNT 13

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

typedef struct Mix_Chunk Mix_Chunk;

enum {
    BLINKY = 0,
    PINKY,
    INKY,
    CLYDE,
    GHOST_COUNT
};

typedef enum {
    ONE_UP = 0,
    HIGH_SCORE_LABEL,
    HIGH_SCORE,
    SCORE,
    CREDITS,
    LIFE1,
    LIFE2,
    LIFE3,
    BONUS_ITEMS,
    READY,
    GAME_OVER_LABEL,
    HUD_ITEM_COUNT
} hud_item_e;

typedef enum {
    MAIN_MENU,
    GET_READY,
    GAME,
    LEVEL_COMPLETE,
    GAME_OVER
} mode_e;

typedef enum {
    SCATTER,
    CHASE,
    FLEE
} ghostmode_e;

typedef struct {
    uint64_t cur_time, prev_time;
    double delta, accumulator;
} gametimer_t;

typedef struct {
    bool show;
    v2f_t pos;
    SDL_Rect srcrect;
} points_sprite_t;

typedef struct {
    bool show;
    int points;
    v2f_t pos;
    SDL_Rect points_sprite;
    char c;
} bonus_t;

typedef enum {
    MUNCH_1 = 0,
    MUNCH_2,
    EAT_GHOST,
    DEATH_1,
    DEATH_2,
    GAME_START,
    RETREAT,
    EAT_FRUIT,
    FLEE_MUSIC,
    MAIN_MUSIC,
    SOUNDS_COUNT,
} sound_e;

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    Mix_Chunk *samples[SOUNDS_COUNT];
    gametimer_t timer;
    points_sprite_t eat_points_sprite;
    bonus_t current_bonus;
    bool show_bonus_points;
    bool up,down,left,right,enter;
    mode_e mode;
    ghostmode_e ghostmode, prev_ghostmode;
    int64_t ghostmode_timer;
    int64_t flee_timer;
    int64_t scene_timer;
    int64_t ghost_eaten_timer;
    int64_t bonus_eaten_timer;
    int64_t blink_timer;
    int64_t blink_interval;
    int64_t game_start_duration;
    int64_t get_ready_duration;
    int64_t game_over_duration;
    int64_t level_complete_duration;
    int ghosts_eaten;   /* number eaten during this FLEE mode, used to calculate points */
    int level;
    int phase;
    bool quit;
    bool just_started;
    int dots_remaining;
    int score;
    float full_speed;
    SDL_Rect completed_board;
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
    GO_HOME,
    NORMAL
} ghost_state_e;

typedef struct {
    v2f_t pos;
    dir_e dir;
    int id;
    int w, h;
    int frame;
    bool show;
    bool moving;
    bool reverse;
    bool frightened;
    int tile;
    float speed;
    char c;            /* character that maps to my spritesheet location */
    int target_tile, scatter_target_tile, respawn_tile;
    ghost_state_e state; 
    int64_t ghost_house_timer; 
    int64_t anim_timer, anim_frame_time;
} ghost_t;

typedef struct {
    v2f_t pos;
    dir_e dir;
    int w, h;
    int frame;
    int delay_frames;
    int lives;
    bool show;
    bool moving;
    bool dead;
    int tile;
    float speed;
    char c;            /* character that maps to my spritesheet location */
    int target_tile; 
    float death_duration; /* in seconds */
    int64_t anim_timer, anim_frame_time, death_timer;
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

void restart_from_death(void);

extern game_t game;
extern ghost_t ghosts[GHOST_COUNT];
extern pacman_t pacman;
extern tilemap_t *tilemap;
extern alphabet_t *alphabet;
extern alphabet_t *alphabet_r;
extern alphabet_t *alphabet_p;
extern alphabet_t *alphabet_o;
extern alphabet_t *alphabet_b;
extern alphabet_t *alphabet_y;
extern bonus_t bonuses[BONUS_COUNT];
extern char board[BOARD_WIDTH*BOARD_HEIGHT];
extern menu_intro_item_t menu_intro_items[MAX_INTRO_ITEMS];
extern sprite_row_t hud_items[HUD_ITEM_COUNT];
extern int high_score;

/* the id (index) of corresponding item in hud_items */
#endif
