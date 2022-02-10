#include <assert.h>
#include <stdio.h>
#include "globals.h"
#include "vecs.h"

bool forward_collide(entity_t *e) {
    /*check tile one ahead in travel direction*/
    int front_tile = get_adjacent_tile(e->tile, e->dir);
 
    if (board[front_tile] == ' ' || board[front_tile] == '.' || board[front_tile] == '0') {
            return false;
    } else {
        v2f_t front_tile_pos = get_tile_pos(front_tile);
        if (vec2f_dist(front_tile_pos, e->pos) > TILE_SIZE)
            return false;
    }

    return true;
}

void update_2pac(void) {
    bool options[4];
    for (int dir=0; dir<4; ++dir) {
        int tile = get_adjacent_tile(pacman.tile, dir);
        options[dir] = !is_solid(board[tile]);
    }

    if (game.left && options[LEFT]) {
        pacman.dir = LEFT;
        pacman.moving = true;
    } else if (game.right && options[RIGHT]) {
        pacman.dir = RIGHT;
        pacman.moving = true;
    } else if (game.up && options[UP]) {
        pacman.dir = UP;
        pacman.moving = true;
    } else if (game.down && options[DOWN]) {
        pacman.dir = DOWN;
        pacman.moving = true;
    }

    if (!pacman.moving) 
        return;

    if (forward_collide(&pacman)) {
        pacman.moving = false;
        return;
    }

    /* move towards target tile */
    pacman.target_tile = get_adjacent_tile(pacman.tile, pacman.dir);
    v2f_t target_tile_pos = get_tile_pos(pacman.target_tile);
    v2f_t dir_vec = vec2f_norm(vec2f_sub(target_tile_pos, pacman.pos));
    pacman.pos = vec2f_add(pacman.pos, vec2f_scale(dir_vec, pacman.speed));

    /* only enter a new tile when your center has crossed the center of the new tile */
    v2f_t cur_tile_pos = get_tile_pos(pacman.tile);

    if (vec2f_dist(cur_tile_pos, pacman.pos) >= TILE_SIZE) {
        int tile_x = (int)(pacman.pos.x + 0.5f) / TILE_SIZE;
        int tile_y = (int)(pacman.pos.y + 0.5f) / TILE_SIZE;
        int next_tile = tile_y * BOARD_WIDTH + tile_x;
        pacman.tile = next_tile;
    }
}
