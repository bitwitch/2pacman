#include <assert.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include "globals.h"
#include "ghosts.h"
#include "stb_ds.h"

menu_intro_item_t menu_intro_items[MAX_INTRO_ITEMS] = {0};
sprite_row_t hud_items[HUD_ITEM_COUNT] = {0};
bonus_t bonuses[BONUS_COUNT] = {0};
tilemap_t *tilemap = NULL;
alphabet_t *alphabet   = NULL;
alphabet_t *alphabet_r = NULL;
alphabet_t *alphabet_p = NULL;
alphabet_t *alphabet_o = NULL;
alphabet_t *alphabet_b = NULL;
alphabet_t *alphabet_y = NULL;

void init_sdl(void) {
    int renderer_flags, window_flags;

    renderer_flags = SDL_RENDERER_ACCELERATED;

    window_flags = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    game.window = SDL_CreateWindow("2pacman", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);

    if (!game.window) {
        printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    game.renderer = SDL_CreateRenderer(game.window, -1, renderer_flags);

    if (!game.renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }

}

/* TODO(shaw): investigate using padding on all sides of the board, and using a
 * stride, this seems like it might make some things easier and cleaner. For
 * example calculating chase targets 
 */

void init_board() {
    assert(BOARD_WIDTH == 28 && BOARD_HEIGHT == 36 && "init_board must be updated if width or height changes");
    memset(board, 0, BOARD_WIDTH*BOARD_HEIGHT);
    int offset = 0;
    memcpy(board+offset, "                            ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "                            ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "                            ", BOARD_WIDTH); offset += BOARD_WIDTH;
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
    memcpy(board+offset, "     {.[]          [].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     {.[] lgvnnbgx [].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "^^^^^7.ip j      k ip.8^^^^^", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "      .   j      k   .      ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "_____6.uo j      k uo.5_____", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     {.[] zhhhhhhc [].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     {.[]          [].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "     {.[] u~~~~~~o [].}     ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "1^^^^7.ip i--da--p ip.8^^^^2", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{............[]............}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.u~~o.u~~~o.[].u~~~o.u~~o.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.i-d].i---p.ip.i---p.[a-p.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{0..[].......  .......[]..0}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "e~o.[].uo.u~~~~~~o.uo.[].u~t", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "r-p.ip.[].i--da--p.[].ip.i-y", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{......[]....[]....[]......}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.u~~~~fs~~o.[].u~~fs~~~~o.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{.i--------p.ip.i--------p.}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "{..........................}", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "4__________________________3", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "                            ", BOARD_WIDTH); offset += BOARD_WIDTH;
    memcpy(board+offset, "                            ", BOARD_WIDTH); offset += BOARD_WIDTH;
}

void init_tilemap(void) {
    SDL_Rect none       = {0, 81, 8, 8};    hmput(tilemap, ' ', none);

    SDL_Rect top_wall   = {8, 0, 8, 8};     hmput(tilemap, '^', top_wall);
    SDL_Rect bot_wall   = {8, 72, 8, 8};    hmput(tilemap, '_', bot_wall);
    SDL_Rect left_wall  = {0, 8, 8, 8};     hmput(tilemap, '{', left_wall);
    SDL_Rect right_wall = {216, 8, 8, 8};   hmput(tilemap, '}', right_wall);
    SDL_Rect otl        = {0, 0, 8, 8};     hmput(tilemap, '1', otl); /* outer top left */
    SDL_Rect otr        = {216, 0, 8, 8};   hmput(tilemap, '2', otr);
    SDL_Rect obr        = {216, 240, 8, 8}; hmput(tilemap, '3', obr);
    SDL_Rect obl        = {0, 240, 8, 8};   hmput(tilemap, '4', obl);
    SDL_Rect itl        = {176, 72, 8, 8};  hmput(tilemap, '5', itl); /* inner top left */
    SDL_Rect itr        = {40, 72, 8, 8};   hmput(tilemap, '6', itr);
    SDL_Rect ibr        = {40, 104, 8, 8};  hmput(tilemap, '7', ibr);
    SDL_Rect ibl        = {176, 104, 8, 8}; hmput(tilemap, '8', ibl);

    SDL_Rect t1 = {104, 0, 8, 8};    hmput(tilemap, 'q', t1);
    SDL_Rect t2 = {112, 0, 8, 8};    hmput(tilemap, 'w', t2);
    SDL_Rect t3 = {0, 192, 8, 8};    hmput(tilemap, 'e', t3);
    SDL_Rect t4 = {0, 200, 8, 8};    hmput(tilemap, 'r', t4);
    SDL_Rect t5 = {216, 192, 8, 8};  hmput(tilemap, 't', t5);
    SDL_Rect t6 = {216, 200, 8, 8};  hmput(tilemap, 'y', t6);

    SDL_Rect bt   = {24, 16, 8, 8};  hmput(tilemap, '~', bt); /* box top */
    SDL_Rect bb   = {24, 32, 8, 8};  hmput(tilemap, '-', bb);
    SDL_Rect bl   = {16, 24, 8, 8};  hmput(tilemap, '[', bl); /* box top */
    SDL_Rect br   = {40, 24, 8, 8};  hmput(tilemap, ']', br);
    SDL_Rect botl = {16, 16, 8, 8};  hmput(tilemap, 'u', botl); /* box outer top left*/
    SDL_Rect bobl = {16, 32, 8, 8};  hmput(tilemap, 'i', bobl);
    SDL_Rect botr = {40, 16, 8, 8};  hmput(tilemap, 'o', botr); 
    SDL_Rect bobr = {40, 32, 8, 8};  hmput(tilemap, 'p', bobr);
    SDL_Rect bitl = {64, 80, 8, 8};  hmput(tilemap, 'a', bitl); /* box inner top left*/
    SDL_Rect bibl = {64, 72, 8, 8};  hmput(tilemap, 's', bibl);
    SDL_Rect bitr = {152, 80, 8, 8}; hmput(tilemap, 'd', bitr); 
    SDL_Rect bibr = {152, 72, 8, 8}; hmput(tilemap, 'f', bibr);

    SDL_Rect ct  = {88, 96, 8, 8};   hmput(tilemap, 'g', ct); /* cage top*/
    SDL_Rect cb  = {88, 128, 8, 8};  hmput(tilemap, 'h', cb);
    SDL_Rect cl  = {80, 104, 8, 8};  hmput(tilemap, 'j', cl); 
    SDL_Rect cr  = {136, 104, 8, 8}; hmput(tilemap, 'k', cr);
    SDL_Rect ctl = {80, 96, 8, 8};   hmput(tilemap, 'l', ctl); /* cage top left*/
    SDL_Rect cbl = {80, 128, 8, 8};  hmput(tilemap, 'z', cbl);
    SDL_Rect ctr = {136, 96, 8, 8};  hmput(tilemap, 'x', ctr); 
    SDL_Rect cbr = {136, 128, 8, 8}; hmput(tilemap, 'c', cbr);
    SDL_Rect cle = {96, 96, 8, 8};   hmput(tilemap, 'v', cle); 
    SDL_Rect cre = {120, 96, 8, 8};  hmput(tilemap, 'b', cre);

    SDL_Rect dot    = {404, 48, 8, 8};    hmput(tilemap, '.', dot);
    SDL_Rect pellet = {388, 48, 8, 8};    hmput(tilemap, '0', pellet);

    SDL_Rect door = {104, 96, 8, 8};      hmput(tilemap, 'n', door);

    SDL_Rect blinky = {228, 64, 16, 16};  hmput(tilemap, 'B', blinky);
    SDL_Rect pinky  = {228, 80, 16, 16};  hmput(tilemap, 'P', pinky);
    SDL_Rect inky   = {228, 96, 16, 16};  hmput(tilemap, 'I', inky);
    SDL_Rect clyde  = {228, 112, 16, 16}; hmput(tilemap, 'C', clyde);

    SDL_Rect cherry = {260, 48, 16, 16};  hmput(tilemap, 'A', cherry);
    SDL_Rect berry  = {276, 48, 16, 16};  hmput(tilemap, 'S', berry);
    SDL_Rect peach  = {292, 48, 16, 16};  hmput(tilemap, 'D', peach);
    SDL_Rect apple  = {308, 48, 16, 16};  hmput(tilemap, 'F', apple);
    SDL_Rect grape  = {324, 48, 16, 16};  hmput(tilemap, 'G', grape);
    SDL_Rect galax  = {340, 48, 16, 16};  hmput(tilemap, 'H', galax);
    SDL_Rect bell   = {356, 48, 16, 16};  hmput(tilemap, 'J', bell);
    SDL_Rect key    = {372, 48, 16, 16};  hmput(tilemap, 'K', key);

    SDL_Rect pacman = {228, 0, 16, 16};  hmput(tilemap, '<', pacman);
    SDL_Rect pacman_left = {244, 16, 16, 16};  hmput(tilemap, '>', pacman_left);

    SDL_Rect board_blue  = {0, 0, 224, 248};   hmput(tilemap, 'L', board_blue);
    SDL_Rect board_white = {452, 0, 224, 248}; hmput(tilemap, 'W', board_white);
}

void init_bonuses(void) {
    bonuses[0]  = (bonus_t){.c = 'A', .points = 100,  .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){372, 32, 16, 16}};
    bonuses[1]  = (bonus_t){.c = 'S', .points = 300,  .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){388, 32, 16, 16}};
    bonuses[2]  = (bonus_t){.c = 'D', .points = 500,  .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){404, 32, 16, 16}};
    bonuses[3]  = (bonus_t){.c = 'D', .points = 500,  .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){404, 32, 16, 16}};
    bonuses[4]  = (bonus_t){.c = 'F', .points = 700,  .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){420, 32, 16, 16}};
    bonuses[5]  = (bonus_t){.c = 'F', .points = 700,  .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){420, 32, 16, 16}};
    bonuses[6]  = (bonus_t){.c = 'G', .points = 1000, .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){420, 48, 22, 16}};
    bonuses[7]  = (bonus_t){.c = 'G', .points = 1000, .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){420, 48, 22, 16}};
    bonuses[8]  = (bonus_t){.c = 'H', .points = 2000, .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){420, 64, 22, 16}};
    bonuses[9]  = (bonus_t){.c = 'H', .points = 2000, .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){420, 64, 22, 16}};
    bonuses[10] = (bonus_t){.c = 'J', .points = 3000, .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){420, 80, 22, 16}};
    bonuses[11] = (bonus_t){.c = 'J', .points = 3000, .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){420, 80, 22, 16}};
    bonuses[12] = (bonus_t){.c = 'K', .points = 5000, .pos = (v2f_t){111.0f, 163.0f},
        .points_sprite = (SDL_Rect){420, 96, 22, 16}};
}

void init_alphabet(void) {
    SDL_Rect a = {228, 128, 8, 8}; hmput(alphabet, 'a', a);
    SDL_Rect b = {236, 128, 8, 8}; hmput(alphabet, 'b', b);
    SDL_Rect c = {244, 128, 8, 8}; hmput(alphabet, 'c', c);
    SDL_Rect d = {252, 128, 8, 8}; hmput(alphabet, 'd', d);
    SDL_Rect e = {260, 128, 8, 8}; hmput(alphabet, 'e', e);
    SDL_Rect f = {268, 128, 8, 8}; hmput(alphabet, 'f', f);
    SDL_Rect g = {276, 128, 8, 8}; hmput(alphabet, 'g', g);
    SDL_Rect h = {284, 128, 8, 8}; hmput(alphabet, 'h', h);
    SDL_Rect i = {292, 128, 8, 8}; hmput(alphabet, 'i', i);
    SDL_Rect j = {300, 128, 8, 8}; hmput(alphabet, 'j', j);
    SDL_Rect k = {308, 128, 8, 8}; hmput(alphabet, 'k', k);
    SDL_Rect l = {316, 128, 8, 8}; hmput(alphabet, 'l', l);
    SDL_Rect m = {324, 128, 8, 8}; hmput(alphabet, 'm', m);
    SDL_Rect n = {332, 128, 8, 8}; hmput(alphabet, 'n', n);
    SDL_Rect o = {340, 128, 8, 8}; hmput(alphabet, 'o', o);
    SDL_Rect p = {348, 128, 8, 8}; hmput(alphabet, 'p', p);
    SDL_Rect q = {356, 128, 8, 8}; hmput(alphabet, 'q', q);
    SDL_Rect r = {364, 128, 8, 8}; hmput(alphabet, 'r', r);
    SDL_Rect s = {372, 128, 8, 8}; hmput(alphabet, 's', s);
    SDL_Rect t = {380, 128, 8, 8}; hmput(alphabet, 't', t);
    SDL_Rect u = {388, 128, 8, 8}; hmput(alphabet, 'u', u);
    SDL_Rect v = {396, 128, 8, 8}; hmput(alphabet, 'v', v);
    SDL_Rect w = {404, 128, 8, 8}; hmput(alphabet, 'w', w);
    SDL_Rect x = {412, 128, 8, 8}; hmput(alphabet, 'x', x);
    SDL_Rect y = {420, 128, 8, 8}; hmput(alphabet, 'y', y);
    SDL_Rect z = {428, 128, 8, 8}; hmput(alphabet, 'z', z);
    SDL_Rect zero  = {228, 136, 8, 8}; hmput(alphabet, '0', zero);
    SDL_Rect one   = {236, 136, 8, 8}; hmput(alphabet, '1', one);
    SDL_Rect two   = {244, 136, 8, 8}; hmput(alphabet, '2', two);
    SDL_Rect three = {252, 136, 8, 8}; hmput(alphabet, '3', three);
    SDL_Rect four  = {260, 136, 8, 8}; hmput(alphabet, '4', four);
    SDL_Rect five  = {268, 136, 8, 8}; hmput(alphabet, '5', five);
    SDL_Rect six   = {276, 136, 8, 8}; hmput(alphabet, '6', six);
    SDL_Rect seven = {284, 136, 8, 8}; hmput(alphabet, '7', seven);
    SDL_Rect eight = {292, 136, 8, 8}; hmput(alphabet, '8', eight);
    SDL_Rect nine  = {300, 136, 8, 8}; hmput(alphabet, '9', nine);
    SDL_Rect dash  = {308, 136, 8, 8}; hmput(alphabet, '-', dash);
    SDL_Rect slash = {316, 136, 8, 8}; hmput(alphabet, '/', slash);
    SDL_Rect quote = {324, 136, 8, 8}; hmput(alphabet, '"', quote);

    a = (SDL_Rect){228, 144, 8, 8}; hmput(alphabet_r, 'a', a);
    b = (SDL_Rect){236, 144, 8, 8}; hmput(alphabet_r, 'b', b);
    c = (SDL_Rect){244, 144, 8, 8}; hmput(alphabet_r, 'c', c);
    d = (SDL_Rect){252, 144, 8, 8}; hmput(alphabet_r, 'd', d);
    e = (SDL_Rect){260, 144, 8, 8}; hmput(alphabet_r, 'e', e);
    f = (SDL_Rect){268, 144, 8, 8}; hmput(alphabet_r, 'f', f);
    g = (SDL_Rect){276, 144, 8, 8}; hmput(alphabet_r, 'g', g);
    h = (SDL_Rect){284, 144, 8, 8}; hmput(alphabet_r, 'h', h);
    i = (SDL_Rect){292, 144, 8, 8}; hmput(alphabet_r, 'i', i);
    j = (SDL_Rect){300, 144, 8, 8}; hmput(alphabet_r, 'j', j);
    k = (SDL_Rect){308, 144, 8, 8}; hmput(alphabet_r, 'k', k);
    l = (SDL_Rect){316, 144, 8, 8}; hmput(alphabet_r, 'l', l);
    m = (SDL_Rect){324, 144, 8, 8}; hmput(alphabet_r, 'm', m);
    n = (SDL_Rect){332, 144, 8, 8}; hmput(alphabet_r, 'n', n);
    o = (SDL_Rect){340, 144, 8, 8}; hmput(alphabet_r, 'o', o);
    p = (SDL_Rect){348, 144, 8, 8}; hmput(alphabet_r, 'p', p);
    q = (SDL_Rect){356, 144, 8, 8}; hmput(alphabet_r, 'q', q);
    r = (SDL_Rect){364, 144, 8, 8}; hmput(alphabet_r, 'r', r);
    s = (SDL_Rect){372, 144, 8, 8}; hmput(alphabet_r, 's', s);
    t = (SDL_Rect){380, 144, 8, 8}; hmput(alphabet_r, 't', t);
    u = (SDL_Rect){388, 144, 8, 8}; hmput(alphabet_r, 'u', u);
    v = (SDL_Rect){396, 144, 8, 8}; hmput(alphabet_r, 'v', v);
    w = (SDL_Rect){404, 144, 8, 8}; hmput(alphabet_r, 'w', w);
    x = (SDL_Rect){412, 144, 8, 8}; hmput(alphabet_r, 'x', x);
    y = (SDL_Rect){420, 144, 8, 8}; hmput(alphabet_r, 'y', y);
    z = (SDL_Rect){428, 144, 8, 8}; hmput(alphabet_r, 'z', z);
    zero  = (SDL_Rect){228, 152, 8, 8}; hmput(alphabet_r, '0', zero);
    one   = (SDL_Rect){236, 152, 8, 8}; hmput(alphabet_r, '1', one);
    two   = (SDL_Rect){244, 152, 8, 8}; hmput(alphabet_r, '2', two);
    three = (SDL_Rect){252, 152, 8, 8}; hmput(alphabet_r, '3', three);
    four  = (SDL_Rect){260, 152, 8, 8}; hmput(alphabet_r, '4', four);
    five  = (SDL_Rect){268, 152, 8, 8}; hmput(alphabet_r, '5', five);
    six   = (SDL_Rect){276, 152, 8, 8}; hmput(alphabet_r, '6', six);
    seven = (SDL_Rect){284, 152, 8, 8}; hmput(alphabet_r, '7', seven);
    eight = (SDL_Rect){292, 152, 8, 8}; hmput(alphabet_r, '8', eight);
    nine  = (SDL_Rect){300, 152, 8, 8}; hmput(alphabet_r, '9', nine);
    dash  = (SDL_Rect){308, 152, 8, 8}; hmput(alphabet_r, '-', dash);
    slash = (SDL_Rect){316, 152, 8, 8}; hmput(alphabet_r, '/', slash);
    quote = (SDL_Rect){324, 152, 8, 8}; hmput(alphabet_r, '"', quote);

    a = (SDL_Rect){228, 160, 8, 8}; hmput(alphabet_p, 'a', a);
    b = (SDL_Rect){236, 160, 8, 8}; hmput(alphabet_p, 'b', b);
    c = (SDL_Rect){244, 160, 8, 8}; hmput(alphabet_p, 'c', c);
    d = (SDL_Rect){252, 160, 8, 8}; hmput(alphabet_p, 'd', d);
    e = (SDL_Rect){260, 160, 8, 8}; hmput(alphabet_p, 'e', e);
    f = (SDL_Rect){268, 160, 8, 8}; hmput(alphabet_p, 'f', f);
    g = (SDL_Rect){276, 160, 8, 8}; hmput(alphabet_p, 'g', g);
    h = (SDL_Rect){284, 160, 8, 8}; hmput(alphabet_p, 'h', h);
    i = (SDL_Rect){292, 160, 8, 8}; hmput(alphabet_p, 'i', i);
    j = (SDL_Rect){300, 160, 8, 8}; hmput(alphabet_p, 'j', j);
    k = (SDL_Rect){308, 160, 8, 8}; hmput(alphabet_p, 'k', k);
    l = (SDL_Rect){316, 160, 8, 8}; hmput(alphabet_p, 'l', l);
    m = (SDL_Rect){324, 160, 8, 8}; hmput(alphabet_p, 'm', m);
    n = (SDL_Rect){332, 160, 8, 8}; hmput(alphabet_p, 'n', n);
    o = (SDL_Rect){340, 160, 8, 8}; hmput(alphabet_p, 'o', o);
    p = (SDL_Rect){348, 160, 8, 8}; hmput(alphabet_p, 'p', p);
    q = (SDL_Rect){356, 160, 8, 8}; hmput(alphabet_p, 'q', q);
    r = (SDL_Rect){364, 160, 8, 8}; hmput(alphabet_p, 'r', r);
    s = (SDL_Rect){372, 160, 8, 8}; hmput(alphabet_p, 's', s);
    t = (SDL_Rect){380, 160, 8, 8}; hmput(alphabet_p, 't', t);
    u = (SDL_Rect){388, 160, 8, 8}; hmput(alphabet_p, 'u', u);
    v = (SDL_Rect){396, 160, 8, 8}; hmput(alphabet_p, 'v', v);
    w = (SDL_Rect){404, 160, 8, 8}; hmput(alphabet_p, 'w', w);
    x = (SDL_Rect){412, 160, 8, 8}; hmput(alphabet_p, 'x', x);
    y = (SDL_Rect){420, 160, 8, 8}; hmput(alphabet_p, 'y', y);
    z = (SDL_Rect){428, 160, 8, 8}; hmput(alphabet_p, 'z', z);
    zero  = (SDL_Rect){228, 168, 8, 8}; hmput(alphabet_p, '0', zero);
    one   = (SDL_Rect){236, 168, 8, 8}; hmput(alphabet_p, '1', one);
    two   = (SDL_Rect){244, 168, 8, 8}; hmput(alphabet_p, '2', two);
    three = (SDL_Rect){252, 168, 8, 8}; hmput(alphabet_p, '3', three);
    four  = (SDL_Rect){260, 168, 8, 8}; hmput(alphabet_p, '4', four);
    five  = (SDL_Rect){268, 168, 8, 8}; hmput(alphabet_p, '5', five);
    six   = (SDL_Rect){276, 168, 8, 8}; hmput(alphabet_p, '6', six);
    seven = (SDL_Rect){284, 168, 8, 8}; hmput(alphabet_p, '7', seven);
    eight = (SDL_Rect){292, 168, 8, 8}; hmput(alphabet_p, '8', eight);
    nine  = (SDL_Rect){300, 168, 8, 8}; hmput(alphabet_p, '9', nine);
    dash  = (SDL_Rect){308, 168, 8, 8}; hmput(alphabet_p, '-', dash);
    slash = (SDL_Rect){316, 168, 8, 8}; hmput(alphabet_p, '/', slash);
    quote = (SDL_Rect){324, 168, 8, 8}; hmput(alphabet_p, '"', quote);

    a = (SDL_Rect){228, 176, 8, 8}; hmput(alphabet_o, 'a', a);
    b = (SDL_Rect){236, 176, 8, 8}; hmput(alphabet_o, 'b', b);
    c = (SDL_Rect){244, 176, 8, 8}; hmput(alphabet_o, 'c', c);
    d = (SDL_Rect){252, 176, 8, 8}; hmput(alphabet_o, 'd', d);
    e = (SDL_Rect){260, 176, 8, 8}; hmput(alphabet_o, 'e', e);
    f = (SDL_Rect){268, 176, 8, 8}; hmput(alphabet_o, 'f', f);
    g = (SDL_Rect){276, 176, 8, 8}; hmput(alphabet_o, 'g', g);
    h = (SDL_Rect){284, 176, 8, 8}; hmput(alphabet_o, 'h', h);
    i = (SDL_Rect){292, 176, 8, 8}; hmput(alphabet_o, 'i', i);
    j = (SDL_Rect){300, 176, 8, 8}; hmput(alphabet_o, 'j', j);
    k = (SDL_Rect){308, 176, 8, 8}; hmput(alphabet_o, 'k', k);
    l = (SDL_Rect){316, 176, 8, 8}; hmput(alphabet_o, 'l', l);
    m = (SDL_Rect){324, 176, 8, 8}; hmput(alphabet_o, 'm', m);
    n = (SDL_Rect){332, 176, 8, 8}; hmput(alphabet_o, 'n', n);
    o = (SDL_Rect){340, 176, 8, 8}; hmput(alphabet_o, 'o', o);
    p = (SDL_Rect){348, 176, 8, 8}; hmput(alphabet_o, 'p', p);
    q = (SDL_Rect){356, 176, 8, 8}; hmput(alphabet_o, 'q', q);
    r = (SDL_Rect){364, 176, 8, 8}; hmput(alphabet_o, 'r', r);
    s = (SDL_Rect){372, 176, 8, 8}; hmput(alphabet_o, 's', s);
    t = (SDL_Rect){380, 176, 8, 8}; hmput(alphabet_o, 't', t);
    u = (SDL_Rect){388, 176, 8, 8}; hmput(alphabet_o, 'u', u);
    v = (SDL_Rect){396, 176, 8, 8}; hmput(alphabet_o, 'v', v);
    w = (SDL_Rect){404, 176, 8, 8}; hmput(alphabet_o, 'w', w);
    x = (SDL_Rect){412, 176, 8, 8}; hmput(alphabet_o, 'x', x);
    y = (SDL_Rect){420, 176, 8, 8}; hmput(alphabet_o, 'y', y);
    z = (SDL_Rect){428, 176, 8, 8}; hmput(alphabet_o, 'z', z);
    zero  = (SDL_Rect){228, 184, 8, 8}; hmput(alphabet_o, '0', zero);
    one   = (SDL_Rect){236, 184, 8, 8}; hmput(alphabet_o, '1', one);
    two   = (SDL_Rect){244, 184, 8, 8}; hmput(alphabet_o, '2', two);
    three = (SDL_Rect){252, 184, 8, 8}; hmput(alphabet_o, '3', three);
    four  = (SDL_Rect){260, 184, 8, 8}; hmput(alphabet_o, '4', four);
    five  = (SDL_Rect){268, 184, 8, 8}; hmput(alphabet_o, '5', five);
    six   = (SDL_Rect){276, 184, 8, 8}; hmput(alphabet_o, '6', six);
    seven = (SDL_Rect){284, 184, 8, 8}; hmput(alphabet_o, '7', seven);
    eight = (SDL_Rect){292, 184, 8, 8}; hmput(alphabet_o, '8', eight);
    nine  = (SDL_Rect){300, 184, 8, 8}; hmput(alphabet_o, '9', nine);
    dash  = (SDL_Rect){308, 184, 8, 8}; hmput(alphabet_o, '-', dash);
    slash = (SDL_Rect){316, 184, 8, 8}; hmput(alphabet_o, '/', slash);
    quote = (SDL_Rect){324, 184, 8, 8}; hmput(alphabet_o, '"', quote);

    a = (SDL_Rect){228, 192, 8, 8}; hmput(alphabet_b, 'a', a);
    b = (SDL_Rect){236, 192, 8, 8}; hmput(alphabet_b, 'b', b);
    c = (SDL_Rect){244, 192, 8, 8}; hmput(alphabet_b, 'c', c);
    d = (SDL_Rect){252, 192, 8, 8}; hmput(alphabet_b, 'd', d);
    e = (SDL_Rect){260, 192, 8, 8}; hmput(alphabet_b, 'e', e);
    f = (SDL_Rect){268, 192, 8, 8}; hmput(alphabet_b, 'f', f);
    g = (SDL_Rect){276, 192, 8, 8}; hmput(alphabet_b, 'g', g);
    h = (SDL_Rect){284, 192, 8, 8}; hmput(alphabet_b, 'h', h);
    i = (SDL_Rect){292, 192, 8, 8}; hmput(alphabet_b, 'i', i);
    j = (SDL_Rect){300, 192, 8, 8}; hmput(alphabet_b, 'j', j);
    k = (SDL_Rect){308, 192, 8, 8}; hmput(alphabet_b, 'k', k);
    l = (SDL_Rect){316, 192, 8, 8}; hmput(alphabet_b, 'l', l);
    m = (SDL_Rect){324, 192, 8, 8}; hmput(alphabet_b, 'm', m);
    n = (SDL_Rect){332, 192, 8, 8}; hmput(alphabet_b, 'n', n);
    o = (SDL_Rect){340, 192, 8, 8}; hmput(alphabet_b, 'o', o);
    p = (SDL_Rect){348, 192, 8, 8}; hmput(alphabet_b, 'p', p);
    q = (SDL_Rect){356, 192, 8, 8}; hmput(alphabet_b, 'q', q);
    r = (SDL_Rect){364, 192, 8, 8}; hmput(alphabet_b, 'r', r);
    s = (SDL_Rect){372, 192, 8, 8}; hmput(alphabet_b, 's', s);
    t = (SDL_Rect){380, 192, 8, 8}; hmput(alphabet_b, 't', t);
    u = (SDL_Rect){388, 192, 8, 8}; hmput(alphabet_b, 'u', u);
    v = (SDL_Rect){396, 192, 8, 8}; hmput(alphabet_b, 'v', v);
    w = (SDL_Rect){404, 192, 8, 8}; hmput(alphabet_b, 'w', w);
    x = (SDL_Rect){412, 192, 8, 8}; hmput(alphabet_b, 'x', x);
    y = (SDL_Rect){420, 192, 8, 8}; hmput(alphabet_b, 'y', y);
    z = (SDL_Rect){428, 192, 8, 8}; hmput(alphabet_b, 'z', z);
    zero  = (SDL_Rect){228, 200, 8, 8}; hmput(alphabet_b, '0', zero);
    one   = (SDL_Rect){236, 200, 8, 8}; hmput(alphabet_b, '1', one);
    two   = (SDL_Rect){244, 200, 8, 8}; hmput(alphabet_b, '2', two);
    three = (SDL_Rect){252, 200, 8, 8}; hmput(alphabet_b, '3', three);
    four  = (SDL_Rect){260, 200, 8, 8}; hmput(alphabet_b, '4', four);
    five  = (SDL_Rect){268, 200, 8, 8}; hmput(alphabet_b, '5', five);
    six   = (SDL_Rect){276, 200, 8, 8}; hmput(alphabet_b, '6', six);
    seven = (SDL_Rect){284, 200, 8, 8}; hmput(alphabet_b, '7', seven);
    eight = (SDL_Rect){292, 200, 8, 8}; hmput(alphabet_b, '8', eight);
    nine  = (SDL_Rect){300, 200, 8, 8}; hmput(alphabet_b, '9', nine);
    dash  = (SDL_Rect){308, 200, 8, 8}; hmput(alphabet_b, '-', dash);
    slash = (SDL_Rect){316, 200, 8, 8}; hmput(alphabet_b, '/', slash);
    quote = (SDL_Rect){324, 200, 8, 8}; hmput(alphabet_b, '"', quote);

    a = (SDL_Rect){228, 208, 8, 8}; hmput(alphabet_y, 'a', a);
    b = (SDL_Rect){236, 208, 8, 8}; hmput(alphabet_y, 'b', b);
    c = (SDL_Rect){244, 208, 8, 8}; hmput(alphabet_y, 'c', c);
    d = (SDL_Rect){252, 208, 8, 8}; hmput(alphabet_y, 'd', d);
    e = (SDL_Rect){260, 208, 8, 8}; hmput(alphabet_y, 'e', e);
    f = (SDL_Rect){268, 208, 8, 8}; hmput(alphabet_y, 'f', f);
    g = (SDL_Rect){276, 208, 8, 8}; hmput(alphabet_y, 'g', g);
    h = (SDL_Rect){284, 208, 8, 8}; hmput(alphabet_y, 'h', h);
    i = (SDL_Rect){292, 208, 8, 8}; hmput(alphabet_y, 'i', i);
    j = (SDL_Rect){300, 208, 8, 8}; hmput(alphabet_y, 'j', j);
    k = (SDL_Rect){308, 208, 8, 8}; hmput(alphabet_y, 'k', k);
    l = (SDL_Rect){316, 208, 8, 8}; hmput(alphabet_y, 'l', l);
    m = (SDL_Rect){324, 208, 8, 8}; hmput(alphabet_y, 'm', m);
    n = (SDL_Rect){332, 208, 8, 8}; hmput(alphabet_y, 'n', n);
    o = (SDL_Rect){340, 208, 8, 8}; hmput(alphabet_y, 'o', o);
    p = (SDL_Rect){348, 208, 8, 8}; hmput(alphabet_y, 'p', p);
    q = (SDL_Rect){356, 208, 8, 8}; hmput(alphabet_y, 'q', q);
    r = (SDL_Rect){364, 208, 8, 8}; hmput(alphabet_y, 'r', r);
    s = (SDL_Rect){372, 208, 8, 8}; hmput(alphabet_y, 's', s);
    t = (SDL_Rect){380, 208, 8, 8}; hmput(alphabet_y, 't', t);
    u = (SDL_Rect){388, 208, 8, 8}; hmput(alphabet_y, 'u', u);
    v = (SDL_Rect){396, 208, 8, 8}; hmput(alphabet_y, 'v', v);
    w = (SDL_Rect){404, 208, 8, 8}; hmput(alphabet_y, 'w', w);
    x = (SDL_Rect){412, 208, 8, 8}; hmput(alphabet_y, 'x', x);
    y = (SDL_Rect){420, 208, 8, 8}; hmput(alphabet_y, 'y', y);
    z = (SDL_Rect){428, 208, 8, 8}; hmput(alphabet_y, 'z', z);
    zero  = (SDL_Rect){228, 216, 8, 8}; hmput(alphabet_y, '0', zero);
    one   = (SDL_Rect){236, 216, 8, 8}; hmput(alphabet_y, '1', one);
    two   = (SDL_Rect){244, 216, 8, 8}; hmput(alphabet_y, '2', two);
    three = (SDL_Rect){252, 216, 8, 8}; hmput(alphabet_y, '3', three);
    four  = (SDL_Rect){260, 216, 8, 8}; hmput(alphabet_y, '4', four);
    five  = (SDL_Rect){268, 216, 8, 8}; hmput(alphabet_y, '5', five);
    six   = (SDL_Rect){276, 216, 8, 8}; hmput(alphabet_y, '6', six);
    seven = (SDL_Rect){284, 216, 8, 8}; hmput(alphabet_y, '7', seven);
    eight = (SDL_Rect){292, 216, 8, 8}; hmput(alphabet_y, '8', eight);
    nine  = (SDL_Rect){300, 216, 8, 8}; hmput(alphabet_y, '9', nine);
    dash  = (SDL_Rect){308, 216, 8, 8}; hmput(alphabet_y, '-', dash);
    slash = (SDL_Rect){316, 216, 8, 8}; hmput(alphabet_y, '/', slash);
    quote = (SDL_Rect){324, 216, 8, 8}; hmput(alphabet_y, '"', quote);
    SDL_Rect excl = {332, 216, 8, 8}; hmput(alphabet_y, '!', excl);

    SDL_Rect space = {0, 88, 8, 8};    hmput(alphabet, ' ', space);
}

void init_hud(void) {
    sprite_row_t *item;

    /* 1UP */
    item = &hud_items[ONE_UP];
    item->show = true;
    item->size = 8;
    item->start_pos.x = 24.0f;
    item->start_pos.y = 0.0f;
    item->srcrects[0] = hmget(alphabet, '1');
    item->srcrects[1] = hmget(alphabet, 'u');
    item->srcrects[2] = hmget(alphabet, 'p');
    item->rect_count = 3;

    /* High Score label*/
    item = &hud_items[HIGH_SCORE_LABEL];
    item->show = true;
    item->size = 8;
    item->start_pos.x = 72.0f;
    item->start_pos.y = 0.0f;
    item->srcrects[0] = hmget(alphabet, 'h');
    item->srcrects[1] = hmget(alphabet, 'i');
    item->srcrects[2] = hmget(alphabet, 'g');
    item->srcrects[3] = hmget(alphabet, 'h');
    item->srcrects[4] = hmget(alphabet, ' ');
    item->srcrects[5] = hmget(alphabet, 's');
    item->srcrects[6] = hmget(alphabet, 'c');
    item->srcrects[7] = hmget(alphabet, 'o');
    item->srcrects[8] = hmget(alphabet, 'r');
    item->srcrects[9] = hmget(alphabet, 'e');
    item->rect_count = 10;

    /* High score */
    item = &hud_items[HIGH_SCORE];
    item->show = true;
    item->size = 8;
    item->start_pos.x = 88.0f;
    item->start_pos.y = 8.0f;
    item->srcrects[0] = hmget(alphabet, ' ');
    item->srcrects[1] = hmget(alphabet, ' ');
    item->srcrects[2] = hmget(alphabet, ' ');
    item->srcrects[3] = hmget(alphabet, ' ');
    item->srcrects[4] = hmget(alphabet, '0');
    item->srcrects[5] = hmget(alphabet, '0');
    item->rect_count = 6;

    /* score */
    item = &hud_items[SCORE];
    item->show = true;
    item->size = 8;
    item->start_pos.x = 8.0f;
    item->start_pos.y = 8.0f;
    item->srcrects[0] = hmget(alphabet, ' ');
    item->srcrects[1] = hmget(alphabet, ' ');
    item->srcrects[2] = hmget(alphabet, ' ');
    item->srcrects[3] = hmget(alphabet, ' ');
    item->srcrects[4] = hmget(alphabet, '0');
    item->srcrects[5] = hmget(alphabet, '0');
    item->rect_count = 6;

    /* credit 1 */
    item = &hud_items[CREDITS];
    item->show = true;
    item->size = 8;
    item->start_pos.x = 24.0f;
    item->start_pos.y = BOARD_HEIGHT*TILE_SIZE - item->size - 2;
    item->srcrects[0] = hmget(alphabet, 'c');
    item->srcrects[1] = hmget(alphabet, 'r');
    item->srcrects[2] = hmget(alphabet, 'e');
    item->srcrects[3] = hmget(alphabet, 'd');
    item->srcrects[4] = hmget(alphabet, 'i');
    item->srcrects[5] = hmget(alphabet, 't');
    item->srcrects[6] = hmget(alphabet, ' ');
    item->srcrects[7] = hmget(alphabet, ' ');
    item->srcrects[8] = hmget(alphabet, '1');
    item->rect_count = 9;

    /* lives */
    item = &hud_items[LIFE1];
    item->show = false;
    item->size = 16;
    item->start_pos.x = 24.0f;
    item->start_pos.y = BOARD_HEIGHT*TILE_SIZE - item->size;
    item->srcrects[0] = hmget(tilemap, '>');
    item->rect_count = 1;

    item = &hud_items[LIFE2];
    item->show = false;
    item->size = 16;
    item->start_pos.x = 24.0f + item->size;
    item->start_pos.y = BOARD_HEIGHT*TILE_SIZE - item->size;
    item->srcrects[0] = hmget(tilemap, '>');
    item->rect_count = 1;

    item = &hud_items[LIFE3];
    item->show = false;
    item->size = 16;
    item->start_pos.x = 24.0f + 2*item->size;
    item->start_pos.y = BOARD_HEIGHT*TILE_SIZE - item->size;
    item->srcrects[0] = hmget(tilemap, '>');
    item->rect_count = 1;

    /* Ready */
    item = &hud_items[READY];
    item->show = false;
    item->size = 8;
    item->start_pos.x = 88.0;
    item->start_pos.y = 160.0;
    item->srcrects[0] = hmget(alphabet_y, 'r');
    item->srcrects[1] = hmget(alphabet_y, 'e');
    item->srcrects[2] = hmget(alphabet_y, 'a');
    item->srcrects[3] = hmget(alphabet_y, 'd');
    item->srcrects[4] = hmget(alphabet_y, 'y');
    item->srcrects[5] = hmget(alphabet_y, '!');
    item->rect_count = 6;

    /* Game Over */
    item = &hud_items[GAME_OVER_LABEL];
    item->show = false;
    item->size = 8;
    item->start_pos.x = 72.0;
    item->start_pos.y = 160.0;
    item->srcrects[0] = hmget(alphabet_r, 'g');
    item->srcrects[1] = hmget(alphabet_r, 'a');
    item->srcrects[2] = hmget(alphabet_r, 'm');
    item->srcrects[3] = hmget(alphabet_r, 'e');
    item->srcrects[4] = hmget(alphabet_r, ' ');
    item->srcrects[5] = hmget(alphabet_r, ' ');
    item->srcrects[6] = hmget(alphabet_r, 'o');
    item->srcrects[7] = hmget(alphabet_r, 'v');
    item->srcrects[8] = hmget(alphabet_r, 'e');
    item->srcrects[9] = hmget(alphabet_r, 'r');
    item->rect_count = 10;

    /* Bonus Items */
    item = &hud_items[BONUS_ITEMS];
    item->show = false;
    item->size = 16;
    item->start_pos.x = 100.0;
    item->start_pos.y = BOARD_HEIGHT*TILE_SIZE - item->size;
    item->srcrects[0] = hmget(tilemap, ' ');
    item->srcrects[1] = hmget(tilemap, ' ');
    item->srcrects[2] = hmget(tilemap, ' ');
    item->srcrects[3] = hmget(tilemap, ' ');
    item->srcrects[4] = hmget(tilemap, ' ');
    item->srcrects[5] = hmget(tilemap, ' ');
    item->srcrects[6] = hmget(tilemap, 'A');
    item->rect_count = 7;

    for (int i=0; i<HUD_ITEM_COUNT; ++i) {
        item = &hud_items[i];
        for (int j=0; j<item->rect_count; ++j) {
            item->dstrects[j].x = (item->start_pos.x + j*item->size)*SCALE;
            item->dstrects[j].y = item->start_pos.y*SCALE;
            item->dstrects[j].w = item->size*SCALE;
            item->dstrects[j].h = item->size*SCALE;
        }
    }
}

void init_menu_intro(void) {
    int count = 0;
    menu_intro_item_t *item;

    /*  character */
    item = &menu_intro_items[count++];
    item->start_time = 12.0f;
    item->size = 8;
    item->start_pos.x = 56.0f;
    item->start_pos.y = 40.0f;
    item->srcrects[0] = hmget(alphabet, 'c');
    item->srcrects[1] = hmget(alphabet, 'h');
    item->srcrects[2] = hmget(alphabet, 'a');
    item->srcrects[3] = hmget(alphabet, 'r');
    item->srcrects[4] = hmget(alphabet, 'a');
    item->srcrects[5] = hmget(alphabet, 'c');
    item->srcrects[6] = hmget(alphabet, 't');
    item->srcrects[7] = hmget(alphabet, 'e');
    item->srcrects[8] = hmget(alphabet, 'r');
    item->srcrects[9] = hmget(alphabet, ' ');
    item->srcrects[10] = hmget(alphabet, '/');
    item->rect_count = 11;

    /*  nickname */
    item = &menu_intro_items[count++];
    item->start_time = 12.0f;
    item->size = 8;
    item->start_pos.x = 56.0f + 12.0f*8.0f;
    item->start_pos.y = 40.0f;
    item->srcrects[0] = hmget(alphabet, 'n');
    item->srcrects[1] = hmget(alphabet, 'i');
    item->srcrects[2] = hmget(alphabet, 'c');
    item->srcrects[3] = hmget(alphabet, 'k');
    item->srcrects[4] = hmget(alphabet, 'n');
    item->srcrects[5] = hmget(alphabet, 'a');
    item->srcrects[6] = hmget(alphabet, 'm');
    item->srcrects[7] = hmget(alphabet, 'e');
    item->rect_count = 8;

    int y_start = 64;

    /* blinky sprite */
    item = &menu_intro_items[count++];
    item->start_time = 11.0f;
    item->size = 16;
    item->start_pos.x = 33.0;
    item->start_pos.y = y_start;
    item->srcrects[0] = hmget(tilemap, ghosts[BLINKY].c);
    item->rect_count = 1;
    /* oikake */
    item = &menu_intro_items[count++];
    item->start_time = 10.5f;
    item->size = 8;
    item->start_pos.x = 64.0;
    item->start_pos.y = y_start+4;
    item->srcrects[0] = hmget(alphabet_r, 'o');
    item->srcrects[1] = hmget(alphabet_r, 'i');
    item->srcrects[2] = hmget(alphabet_r, 'k');
    item->srcrects[3] = hmget(alphabet_r, 'a');
    item->srcrects[4] = hmget(alphabet_r, 'k');
    item->srcrects[5] = hmget(alphabet_r, 'e');
    item->srcrects[6] = hmget(alphabet_r, '-');
    item->srcrects[7] = hmget(alphabet_r, '-');
    item->srcrects[8] = hmget(alphabet_r, '-');
    item->srcrects[9] = hmget(alphabet_r, '-');
    item->rect_count = 10;
    /* akabei */
    item = &menu_intro_items[count++];
    item->start_time = 10;
    item->size = 8;
    item->start_pos.x = 64.0 + 8*10;
    item->start_pos.y = y_start+4;
    item->srcrects[0] = hmget(alphabet_r, '"');
    item->srcrects[1] = hmget(alphabet_r, 'a');
    item->srcrects[2] = hmget(alphabet_r, 'k');
    item->srcrects[3] = hmget(alphabet_r, 'a');
    item->srcrects[4] = hmget(alphabet_r, 'b');
    item->srcrects[5] = hmget(alphabet_r, 'e');
    item->srcrects[6] = hmget(alphabet_r, 'i');
    item->srcrects[7] = hmget(alphabet_r, '"');
    item->rect_count = 8;


    /* pinky sprite */
    item = &menu_intro_items[count++];
    item->start_time = 9.0f;
    item->size = 16;
    item->start_pos.x = 33.0;
    item->start_pos.y = y_start+16+12;
    item->srcrects[0] = hmget(tilemap, ghosts[PINKY].c);
    item->rect_count = 1;
    /* machibuse */
    item = &menu_intro_items[count++];
    item->start_time = 8.5f;
    item->size = 8;
    item->start_pos.x = 64.0;
    item->start_pos.y = y_start+16+12+4;
    item->srcrects[0]  = hmget(alphabet_p, 'm');
    item->srcrects[1]  = hmget(alphabet_p, 'a');
    item->srcrects[2]  = hmget(alphabet_p, 'c');
    item->srcrects[3]  = hmget(alphabet_p, 'h');
    item->srcrects[4]  = hmget(alphabet_p, 'i');
    item->srcrects[5]  = hmget(alphabet_p, 'b');
    item->srcrects[6]  = hmget(alphabet_p, 'u');
    item->srcrects[7]  = hmget(alphabet_p, 's');
    item->srcrects[8]  = hmget(alphabet_p, 'e');
    item->srcrects[9]  = hmget(alphabet_p, '-');
    item->srcrects[10] = hmget(alphabet_p, '-');
    item->rect_count = 11;
    /* pinky */
    item = &menu_intro_items[count++];
    item->start_time = 8.0f;
    item->size = 8;
    item->start_pos.x = 64.0 + 8*11;
    item->start_pos.y = y_start+16+12+4;
    item->srcrects[0] = hmget(alphabet_p, '"');
    item->srcrects[1] = hmget(alphabet_p, 'p');
    item->srcrects[2] = hmget(alphabet_p, 'i');
    item->srcrects[3] = hmget(alphabet_p, 'n');
    item->srcrects[4] = hmget(alphabet_p, 'k');
    item->srcrects[5] = hmget(alphabet_p, 'y');
    item->srcrects[6] = hmget(alphabet_p, '"');
    item->rect_count = 7;


    /* inky sprite */
    item = &menu_intro_items[count++];
    item->start_time = 7.0f;
    item->size = 16;
    item->start_pos.x = 33.0;
    item->start_pos.y = y_start + 2*16 + 2*12;
    item->srcrects[0] = hmget(tilemap, ghosts[INKY].c);
    item->rect_count = 1;
    /* kimagure */
    item = &menu_intro_items[count++];
    item->start_time = 6.5f;
    item->size = 8;
    item->start_pos.x = 64.0;
    item->start_pos.y = y_start + 2*16 + 2*12 + 4;
    item->srcrects[0] = hmget(alphabet_b, 'k');
    item->srcrects[1] = hmget(alphabet_b, 'i');
    item->srcrects[2] = hmget(alphabet_b, 'm');
    item->srcrects[3] = hmget(alphabet_b, 'a');
    item->srcrects[4] = hmget(alphabet_b, 'g');
    item->srcrects[5] = hmget(alphabet_b, 'u');
    item->srcrects[6] = hmget(alphabet_b, 'r');
    item->srcrects[7] = hmget(alphabet_b, 'e');
    item->srcrects[8] = hmget(alphabet_b, '-');
    item->srcrects[9] = hmget(alphabet_b, '-');
    item->rect_count = 10;
    /* aosuke */
    item = &menu_intro_items[count++];
    item->start_time = 6.0f;
    item->size = 8;
    item->start_pos.x = 64.0 + 8*10;
    item->start_pos.y = y_start + 2*16 + 2*12 + 4;
    item->srcrects[0] = hmget(alphabet_b, '"');
    item->srcrects[1] = hmget(alphabet_b, 'a');
    item->srcrects[2] = hmget(alphabet_b, 'o');
    item->srcrects[3] = hmget(alphabet_b, 's');
    item->srcrects[4] = hmget(alphabet_b, 'u');
    item->srcrects[5] = hmget(alphabet_b, 'k');
    item->srcrects[6] = hmget(alphabet_b, 'e');
    item->srcrects[7] = hmget(alphabet_b, '"');
    item->rect_count = 8;


    /* clyde sprite */
    item = &menu_intro_items[count++];
    item->start_time = 5.0f;
    item->size = 16;
    item->start_pos.x = 33.0;
    item->start_pos.y = y_start + 3*16 + 3*12;
    item->srcrects[0] = hmget(tilemap, ghosts[CLYDE].c);
    item->rect_count = 1;
    /* otoboke */
    item = &menu_intro_items[count++];
    item->start_time = 4.5f;
    item->size = 8;
    item->start_pos.x = 64.0;
    item->start_pos.y = y_start + 3*16 + 3*12 + 4;
    item->srcrects[0] = hmget(alphabet_o, 'o');
    item->srcrects[1] = hmget(alphabet_o, 't');
    item->srcrects[2] = hmget(alphabet_o, 'o');
    item->srcrects[3] = hmget(alphabet_o, 'b');
    item->srcrects[4] = hmget(alphabet_o, 'o');
    item->srcrects[5] = hmget(alphabet_o, 'k');
    item->srcrects[6] = hmget(alphabet_o, 'e');
    item->srcrects[7] = hmget(alphabet_o, '-');
    item->srcrects[8] = hmget(alphabet_o, '-');
    item->srcrects[9] = hmget(alphabet_o, '-');
    item->rect_count = 10;
    /* guzuta */
    item = &menu_intro_items[count++];
    item->start_time = 4.0f;
    item->size = 8;
    item->start_pos.x = 64.0 + 8*10;
    item->start_pos.y = y_start + 3*16 + 3*12 + 4;
    item->srcrects[0] = hmget(alphabet_o, '"');
    item->srcrects[1] = hmget(alphabet_o, 'g');
    item->srcrects[2] = hmget(alphabet_o, 'u');
    item->srcrects[3] = hmget(alphabet_o, 'z');
    item->srcrects[4] = hmget(alphabet_o, 'u');
    item->srcrects[5] = hmget(alphabet_o, 't');
    item->srcrects[6] = hmget(alphabet_o, 'a');
    item->srcrects[7] = hmget(alphabet_o, '"');
    item->rect_count = 8;

    /* Press Enter */
    item = &menu_intro_items[count++];
    item->blink = true;
    item->start_time = 3.5f;
    item->size = 8;
    item->start_pos.x = 42.0;
    item->start_pos.y = y_start + 5*16 + 5*12;
    item->srcrects[0]  = hmget(alphabet, '-');
    item->srcrects[1]  = hmget(alphabet, '-');
    item->srcrects[2]  = hmget(alphabet, ' ');
    item->srcrects[3]  = hmget(alphabet, 'p');
    item->srcrects[4]  = hmget(alphabet, 'r');
    item->srcrects[5]  = hmget(alphabet, 'e');
    item->srcrects[6]  = hmget(alphabet, 's');
    item->srcrects[7]  = hmget(alphabet, 's');
    item->srcrects[8]  = hmget(alphabet, ' ');
    item->srcrects[9]  = hmget(alphabet, 'e');
    item->srcrects[10] = hmget(alphabet, 'n');
    item->srcrects[11] = hmget(alphabet, 't');
    item->srcrects[12] = hmget(alphabet, 'e');
    item->srcrects[13] = hmget(alphabet, 'r');
    item->srcrects[14] = hmget(alphabet, ' ');
    item->srcrects[15] = hmget(alphabet, '-');
    item->srcrects[16] = hmget(alphabet, '-');
    item->rect_count = 17;


    for (int i=0; i<count; ++i) {
        item = &menu_intro_items[i];
        for (int j=0; j<item->rect_count; ++j) {
            item->dstrects[j].x = (item->start_pos.x + j*item->size)*SCALE;
            item->dstrects[j].y = item->start_pos.y*SCALE;
            item->dstrects[j].w = item->size*SCALE;
            item->dstrects[j].h = item->size*SCALE;
        }
    }

}

void reset_pacman(void) {
    pacman.pos.x = 111.0f;
    pacman.pos.y = 212.0f;
    pacman.tile = tile_at(pacman.pos);
    pacman.dir = LEFT;
    pacman.moving = true;
    pacman.anim_timer = pacman.anim_frame_time;
}

void init_pacman(void) {
    pacman.c = '<';
    pacman.show = true;
    pacman.pos.x = 111.0f;
    pacman.pos.y = 212.0f;
    pacman.w = 16;
    pacman.h = 16;
    pacman.tile = tile_at(pacman.pos);
    /*pacman.speed = 0.814159;*/
    pacman.speed = 0.814159;
    pacman.dir = LEFT;
    pacman.moving = true;
    pacman.lives = 3;
    pacman.frame = 0;
    pacman.anim_frame_time = (int64_t)SEC_TO_USEC / 15;
    pacman.anim_timer = pacman.anim_frame_time;
    pacman.dead = false;
    pacman.death_duration = 1;
}

void init_ghosts(void) {
    float x,y;
    float speed = 0.814159;

    for (int i=0; i<GHOST_COUNT; ++i) {
        ghosts[i].show = true;
        ghosts[i].w = 16;
        ghosts[i].h = 16;
        ghosts[i].speed = speed;
        ghosts[i].moving = true;
        ghosts[i].anim_frame_time = (int64_t)SEC_TO_USEC / 15;
        ghosts[i].anim_timer = ghosts[i].anim_frame_time;
    }

    x = 111.0f; y = 116.0f;
    ghosts[BLINKY].id = BLINKY;
    ghosts[BLINKY].c = 'B';
    ghosts[BLINKY].pos.x = x; 
    ghosts[BLINKY].pos.y = y;
    ghosts[BLINKY].tile = tile_at(ghosts[BLINKY].pos);
    ghosts[BLINKY].dir = LEFT;
    ghosts[BLINKY].state = NORMAL;
    ghosts[BLINKY].scatter_target_tile = 25;
    ghosts[BLINKY].respawn_tile = 490;

    x = 111.0f; y = 140.0f;
    ghosts[PINKY].id = PINKY;
    ghosts[PINKY].c = 'P';
    ghosts[PINKY].pos.x = x; 
    ghosts[PINKY].pos.y = y;
    ghosts[PINKY].tile = tile_at(ghosts[PINKY].pos);
    ghosts[PINKY].dir = UP;
    ghosts[PINKY].state = HOUSE_PARTY;
    ghosts[PINKY].ghost_house_timer = 1 * SEC_TO_USEC;
    ghosts[PINKY].scatter_target_tile = 2;
    ghosts[PINKY].respawn_tile = 490;

    x = 95.0f; y = 140.0f;
    ghosts[INKY].id = INKY;
    ghosts[INKY].c = 'I';
    ghosts[INKY].pos.x = x; 
    ghosts[INKY].pos.y = y;
    ghosts[INKY].tile = tile_at(ghosts[INKY].pos);
    ghosts[INKY].dir = UP;
    ghosts[INKY].ghost_house_timer = 4 * SEC_TO_USEC;
    ghosts[INKY].state = HOUSE_PARTY;
    ghosts[INKY].scatter_target_tile = (BOARD_HEIGHT-1)*BOARD_WIDTH + (BOARD_WIDTH-1);
    ghosts[INKY].respawn_tile = 489;

    x = 127.0f; y = 140.0f;
    ghosts[CLYDE].id = CLYDE;
    ghosts[CLYDE].c = 'C';
    ghosts[CLYDE].pos.x = x; 
    ghosts[CLYDE].pos.y = y;
    ghosts[CLYDE].tile = tile_at(ghosts[CLYDE].pos);
    ghosts[CLYDE].dir = UP;
    ghosts[CLYDE].ghost_house_timer = 7 * SEC_TO_USEC;
    ghosts[CLYDE].state = HOUSE_PARTY;
    ghosts[CLYDE].scatter_target_tile = (BOARD_HEIGHT-1)*BOARD_WIDTH;
    ghosts[CLYDE].respawn_tile = 491;
}


