#ifndef __VECS_H__
#define __VECS_H__

typedef struct {
    float x,y;
} v2f_t;

float vec2f_dist(v2f_t v1, v2f_t v2);
v2f_t get_tile_pos(int tile);
float dist_tiles(int tile1, int tile2);
int get_adjacent_tile(int tile, int dir);
bool is_solid(char c);

#endif

