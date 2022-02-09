#include <assert.h>
#include <math.h>
#include "globals.h"
#include "vecs.h"

float vec2f_dist(v2f_t v1, v2f_t v2) {
    float a = fabs(v2.x - v1.x);
    float b = fabs(v2.y - v1.y);
    return sqrt(a*a + b*b);
}



/* TODO(shaw): all these tile functions should probably move to a different file */

/* get position of the center point of a tile */
v2f_t get_tile_pos(int tile) {
     v2f_t tile_pos = { 
        .x = (tile % BOARD_WIDTH) * TILE_SIZE + 0.5f*TILE_SIZE, 
        .y = (tile / BOARD_WIDTH) * TILE_SIZE + 0.5f*TILE_SIZE 
     };
     return tile_pos;
}


float dist_tiles(int tile1, int tile2) {
    v2f_t tile1_pos = get_tile_pos(tile1);
    v2f_t tile2_pos = get_tile_pos(tile2);
    return vec2f_dist(tile1_pos, tile2_pos);
}

int get_adjacent_tile(int tile, int dir) {
    int adj;
    if (dir == LEFT)       adj = tile - 1;
    else if (dir == RIGHT) adj = tile + 1;
    else if (dir == UP)    adj = tile - BOARD_WIDTH;
    else if (dir == DOWN)  adj = tile + BOARD_WIDTH;
    else
        assert(false && "unknown direction in get_adjacent_tile");
    return adj;
}

bool is_solid(char c) {
    return (c != ' ' && c != '.' && c != 'O' && c !='n');
}
