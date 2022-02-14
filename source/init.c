#include <assert.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include "globals.h"
#include "ghosts.h"
#include "stb_ds.h"

menu_intro_item_t menu_intro_items[12];

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

void init_board(char *board) {
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
    SDL_Rect none       = {0, 88, 8, 8};    hmput(tilemap, ' ', none);

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
    SDL_Rect pacman = {228, 0, 16, 16};  hmput(tilemap, '>', pacman);
}

void init_alphabet(void) {
    SDL_Rect a = {228, 56, 8, 8}; hmput(alphabet, 'a', a);
    SDL_Rect b = {236, 56, 8, 8}; hmput(alphabet, 'b', b);
    SDL_Rect c = {244, 56, 8, 8}; hmput(alphabet, 'c', c);
    SDL_Rect d = {252, 56, 8, 8}; hmput(alphabet, 'd', d);
    SDL_Rect e = {260, 56, 8, 8}; hmput(alphabet, 'e', e);
    SDL_Rect f = {268, 56, 8, 8}; hmput(alphabet, 'f', f);
    SDL_Rect g = {276, 56, 8, 8}; hmput(alphabet, 'g', g);
    SDL_Rect h = {284, 56, 8, 8}; hmput(alphabet, 'h', h);
    SDL_Rect i = {292, 56, 8, 8}; hmput(alphabet, 'i', i);
    SDL_Rect j = {300, 56, 8, 8}; hmput(alphabet, 'j', j);
    SDL_Rect k = {308, 56, 8, 8}; hmput(alphabet, 'k', k);
    SDL_Rect l = {316, 56, 8, 8}; hmput(alphabet, 'l', l);
    SDL_Rect m = {324, 56, 8, 8}; hmput(alphabet, 'm', m);
    SDL_Rect n = {332, 56, 8, 8}; hmput(alphabet, 'n', n);
    SDL_Rect o = {340, 56, 8, 8}; hmput(alphabet, 'o', o);
    SDL_Rect p = {356, 56, 8, 8}; hmput(alphabet, 'p', p);
    SDL_Rect q = {364, 56, 8, 8}; hmput(alphabet, 'q', q);
    SDL_Rect r = {372, 56, 8, 8}; hmput(alphabet, 'r', r);
    SDL_Rect s = {380, 56, 8, 8}; hmput(alphabet, 's', s);
    SDL_Rect t = {388, 56, 8, 8}; hmput(alphabet, 't', t);
    SDL_Rect u = {396, 56, 8, 8}; hmput(alphabet, 'u', u);
    SDL_Rect v = {404, 56, 8, 8}; hmput(alphabet, 'v', v);
    SDL_Rect w = {412, 56, 8, 8}; hmput(alphabet, 'w', w);
    SDL_Rect x = {420, 56, 8, 8}; hmput(alphabet, 'x', x);
    SDL_Rect y = {428, 56, 8, 8}; hmput(alphabet, 'y', y);
    SDL_Rect z = {436, 56, 8, 8}; hmput(alphabet, 'z', z);
    SDL_Rect dash  = {444, 56, 8, 8}; hmput(alphabet, '-', dash);
    SDL_Rect quote = {452, 56, 8, 8}; hmput(alphabet, '"', quote);
}

void init_menu_intro(void) {
    assert(ARRAY_COUNT(menu_intro_items) == 12 && "inside init_menu_intro");

    int i = 0;
    int y_start = 64;
    menu_intro_item_t *item;

    /* blinky sprite */
    item = &menu_intro_items[i++];
    item->start_time = 11.0f;
    item->size = 16;
    item->start_pos.x = 24.0;
    item->start_pos.y = y_start;
    item->srcrects[0] = hmget(tilemap, ghosts[BLINKY].c);
    item->rect_count = 1;
    /* oikake */
    item = &menu_intro_items[i++];
    item->start_time = 10.5f;
    item->size = 8;
    item->start_pos.x = 72.0;
    item->start_pos.y = y_start+8;
    item->srcrects[0] = hmget(alphabet, 'o');
    item->srcrects[1] = hmget(alphabet, 'i');
    item->srcrects[2] = hmget(alphabet, 'k');
    item->srcrects[3] = hmget(alphabet, 'a');
    item->srcrects[4] = hmget(alphabet, 'k');
    item->srcrects[5] = hmget(alphabet, 'e');
    item->srcrects[6] = hmget(alphabet, '-');
    item->srcrects[7] = hmget(alphabet, '-');
    item->srcrects[8] = hmget(alphabet, '-');
    item->srcrects[9] = hmget(alphabet, '-');
    item->rect_count = 10.0f;
    /* akabei */
    item = &menu_intro_items[i++];
    item->start_time = 10;
    item->size = 8;
    item->start_pos.x = 72.0 + 8*10;
    item->start_pos.y = y_start+8;
    item->srcrects[0] = hmget(alphabet, '"');
    item->srcrects[1] = hmget(alphabet, 'a');
    item->srcrects[2] = hmget(alphabet, 'k');
    item->srcrects[3] = hmget(alphabet, 'a');
    item->srcrects[4] = hmget(alphabet, 'b');
    item->srcrects[5] = hmget(alphabet, 'e');
    item->srcrects[6] = hmget(alphabet, 'i');
    item->srcrects[7] = hmget(alphabet, '"');
    item->rect_count = 8;


    /* pinky sprite */
    item = &menu_intro_items[i++];
    item->start_time = 9.0f;
    item->size = 16;
    item->start_pos.x = 24.0;
    item->start_pos.y = y_start+16+12;
    item->srcrects[0] = hmget(tilemap, ghosts[PINKY].c);
    item->rect_count = 1;
    /* machibuse */
    item = &menu_intro_items[i++];
    item->start_time = 8.5f;
    item->size = 8;
    item->start_pos.x = 72.0;
    item->start_pos.y = y_start+16+12+8;
    item->srcrects[0]  = hmget(alphabet, 'm');
    item->srcrects[1]  = hmget(alphabet, 'a');
    item->srcrects[2]  = hmget(alphabet, 'c');
    item->srcrects[3]  = hmget(alphabet, 'h');
    item->srcrects[4]  = hmget(alphabet, 'i');
    item->srcrects[5]  = hmget(alphabet, 'b');
    item->srcrects[6]  = hmget(alphabet, 'u');
    item->srcrects[7]  = hmget(alphabet, 's');
    item->srcrects[8]  = hmget(alphabet, 'e');
    item->srcrects[9]  = hmget(alphabet, '-');
    item->srcrects[10] = hmget(alphabet, '-');
    item->rect_count = 11;
    /* pinky */
    item = &menu_intro_items[i++];
    item->start_time = 8.0f;
    item->size = 8;
    item->start_pos.x = 72.0 + 8*11;
    item->start_pos.y = y_start+16+12+8;
    item->srcrects[0] = hmget(alphabet, '"');
    item->srcrects[1] = hmget(alphabet, 'p');
    item->srcrects[2] = hmget(alphabet, 'i');
    item->srcrects[3] = hmget(alphabet, 'n');
    item->srcrects[4] = hmget(alphabet, 'k');
    item->srcrects[5] = hmget(alphabet, 'y');
    item->srcrects[6] = hmget(alphabet, '"');
    item->rect_count = 7;


    /* inky sprite */
    item = &menu_intro_items[i++];
    item->start_time = 7.0f;
    item->size = 16;
    item->start_pos.x = 24.0;
    item->start_pos.y = y_start + 2*16 + 2*12;
    item->srcrects[0] = hmget(tilemap, ghosts[INKY].c);
    item->rect_count = 1;
    /* kimagure */
    item = &menu_intro_items[i++];
    item->start_time = 6.5f;
    item->size = 8;
    item->start_pos.x = 72.0;
    item->start_pos.y = y_start + 2*16 + 2*12 + 8;
    item->srcrects[0] = hmget(alphabet, 'k');
    item->srcrects[1] = hmget(alphabet, 'i');
    item->srcrects[2] = hmget(alphabet, 'm');
    item->srcrects[3] = hmget(alphabet, 'a');
    item->srcrects[4] = hmget(alphabet, 'g');
    item->srcrects[5] = hmget(alphabet, 'u');
    item->srcrects[6] = hmget(alphabet, 'r');
    item->srcrects[7] = hmget(alphabet, 'e');
    item->srcrects[8] = hmget(alphabet, '-');
    item->srcrects[9] = hmget(alphabet, '-');
    item->rect_count = 10;
    /* aosuke */
    item = &menu_intro_items[i++];
    item->start_time = 6.0f;
    item->size = 8;
    item->start_pos.x = 72.0 + 8*10;
    item->start_pos.y = y_start + 2*16 + 2*12 + 8;
    item->srcrects[0] = hmget(alphabet, '"');
    item->srcrects[1] = hmget(alphabet, 'a');
    item->srcrects[2] = hmget(alphabet, 'o');
    item->srcrects[3] = hmget(alphabet, 's');
    item->srcrects[4] = hmget(alphabet, 'u');
    item->srcrects[5] = hmget(alphabet, 'k');
    item->srcrects[6] = hmget(alphabet, 'e');
    item->srcrects[7] = hmget(alphabet, '"');
    item->rect_count = 8;


    /* clyde sprite */
    item = &menu_intro_items[i++];
    item->start_time = 5.0f;
    item->size = 16;
    item->start_pos.x = 24.0;
    item->start_pos.y = y_start + 3*16 + 3*12;
    item->srcrects[0] = hmget(tilemap, ghosts[CLYDE].c);
    item->rect_count = 1;
    /* otoboke */
    item = &menu_intro_items[i++];
    item->start_time = 4.5f;
    item->size = 8;
    item->start_pos.x = 72.0;
    item->start_pos.y = y_start + 3*16 + 3*12 + 8;
    item->srcrects[0] = hmget(alphabet, 'o');
    item->srcrects[1] = hmget(alphabet, 't');
    item->srcrects[2] = hmget(alphabet, 'o');
    item->srcrects[3] = hmget(alphabet, 'b');
    item->srcrects[4] = hmget(alphabet, 'o');
    item->srcrects[5] = hmget(alphabet, 'k');
    item->srcrects[6] = hmget(alphabet, 'e');
    item->srcrects[7] = hmget(alphabet, '-');
    item->srcrects[8] = hmget(alphabet, '-');
    item->srcrects[9] = hmget(alphabet, '-');
    item->rect_count = 10;
    /* guzuta */
    item = &menu_intro_items[i++];
    item->start_time = 4.0f;
    item->size = 8;
    item->start_pos.x = 72.0 + 8*10;
    item->start_pos.y = y_start + 3*16 + 3*12 + 8;
    item->srcrects[0] = hmget(alphabet, '"');
    item->srcrects[1] = hmget(alphabet, 'g');
    item->srcrects[2] = hmget(alphabet, 'u');
    item->srcrects[3] = hmget(alphabet, 'z');
    item->srcrects[4] = hmget(alphabet, 'u');
    item->srcrects[5] = hmget(alphabet, 't');
    item->srcrects[6] = hmget(alphabet, 'a');
    item->srcrects[7] = hmget(alphabet, '"');
    item->rect_count = 8;


    for (i=0; i<ARRAY_COUNT(menu_intro_items); ++i) {
        item = &menu_intro_items[i];
        for (int j=0; j<12; ++j) {
            item->dstrects[j].x = (item->start_pos.x + j*item->size)*SCALE;
            item->dstrects[j].y = item->start_pos.y*SCALE;
            item->dstrects[j].w = item->size*SCALE;
            item->dstrects[j].h = item->size*SCALE;
        }
    }
}


void init_entities(void) {
    float x,y;
    float speed = 0.814159;

    x = 111.0f; y = 116.0f;
    ghosts[BLINKY].c = 'B';
    ghosts[BLINKY].pos.x = x; 
    ghosts[BLINKY].pos.y = y;
    ghosts[BLINKY].w = 16;
    ghosts[BLINKY].h = 16;
    ghosts[BLINKY].tile = tile_at(ghosts[BLINKY].pos);
    ghosts[BLINKY].speed = speed;
    ghosts[BLINKY].dir = LEFT;
    ghosts[BLINKY].moving = true;
    ghosts[BLINKY].state = NORMAL;
    ghosts[BLINKY].scatter_target_tile = 25;
    ghosts[BLINKY].anim_frame_time = (int64_t)SEC_TO_USEC / 15;
    ghosts[BLINKY].anim_timer = ghosts[BLINKY].anim_frame_time;

    x = 95.0f; y = 140.0f;
    ghosts[INKY].c = 'I';
    ghosts[INKY].pos.x = x; 
    ghosts[INKY].pos.y = y;
    ghosts[INKY].w = 16;
    ghosts[INKY].h = 16;
    ghosts[INKY].tile = tile_at(ghosts[INKY].pos);
    ghosts[INKY].speed = speed;
    ghosts[INKY].dir = UP;
    ghosts[INKY].ghost_house_timer = 4 * SEC_TO_USEC;
    ghosts[INKY].moving = true;
    ghosts[INKY].state = HOUSE_PARTY;
    ghosts[INKY].scatter_target_tile = 36*BOARD_WIDTH;
    ghosts[INKY].anim_frame_time = (int64_t)SEC_TO_USEC / 15;
    ghosts[INKY].anim_timer = ghosts[INKY].anim_frame_time;

    x = 111.0f; y = 140.0f;
    ghosts[PINKY].c = 'P';
    ghosts[PINKY].pos.x = x; 
    ghosts[PINKY].pos.y = y;
    ghosts[PINKY].w = 16;
    ghosts[PINKY].h = 16;
    ghosts[PINKY].tile = tile_at(ghosts[PINKY].pos);
    ghosts[PINKY].speed = speed;
    ghosts[PINKY].dir = UP;
    ghosts[PINKY].state = HOUSE_PARTY;
    ghosts[PINKY].ghost_house_timer = 1 * SEC_TO_USEC;
    ghosts[PINKY].moving = true;
    ghosts[PINKY].scatter_target_tile = 2;
    ghosts[PINKY].anim_frame_time = (int64_t)SEC_TO_USEC / 15;
    ghosts[PINKY].anim_timer = ghosts[PINKY].anim_frame_time;

    x = 127.0f; y = 140.0f;
    ghosts[CLYDE].c = 'C';
    ghosts[CLYDE].pos.x = x; 
    ghosts[CLYDE].pos.y = y;
    ghosts[CLYDE].w = 16;
    ghosts[CLYDE].h = 16;
    ghosts[CLYDE].tile = tile_at(ghosts[CLYDE].pos);
    ghosts[CLYDE].speed = speed;
    ghosts[CLYDE].dir = UP;
    ghosts[CLYDE].state = HOUSE_PARTY;
    ghosts[CLYDE].ghost_house_timer = 7 * SEC_TO_USEC;
    ghosts[CLYDE].moving = true;
    ghosts[CLYDE].scatter_target_tile = 36*BOARD_WIDTH + 27;
    ghosts[CLYDE].anim_frame_time = (int64_t)SEC_TO_USEC / (int64_t)15;
    ghosts[CLYDE].anim_timer = ghosts[CLYDE].anim_frame_time;

    x = 111.0f; y = 212.0f;
    pacman.c = '>';
    pacman.pos.x = x;
    pacman.pos.y = y;
    pacman.w = 16;
    pacman.h = 16;
    pacman.tile = tile_at(pacman.pos);
    pacman.speed = speed;
    pacman.dir = LEFT;
    pacman.moving = true;
    pacman.anim_frame_time = (int64_t)SEC_TO_USEC / 15;
    pacman.anim_timer = pacman.anim_frame_time;
}


