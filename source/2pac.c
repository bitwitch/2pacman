#include <assert.h>
#include <stdio.h>
#include "globals.h"
#include "vecs.h"

bool canmove(entity_t *e) {
    /*check tile one ahead in travel direction*/
    int front_tile;

    if (e->dir == LEFT)
        front_tile = e->tile-1;
    else if (e->dir == RIGHT)
        front_tile = e->tile+1;
    else if (e->dir == UP)
        front_tile = e->tile-BOARD_WIDTH;
    else if (e->dir == DOWN)
        front_tile = e->tile+BOARD_WIDTH;
    else
        assert(false && "unknown direction in canmove");
 
    if (board[front_tile] == ' ' || board[front_tile] == '.' || board[front_tile] == '0') {
            return true;
    } else {
        v2f_t front_tile_pos = { 
            .x = (front_tile % BOARD_WIDTH) * TILE_SIZE + 0.5f*TILE_SIZE, 
            .y = (front_tile / BOARD_WIDTH) * TILE_SIZE + 0.5f*TILE_SIZE };
        if (vec2f_dist(front_tile_pos, e->pos) > TILE_SIZE)
            return true;
    }

    return false;
}

void update_2pac(void) {
    if (game.left && !game.right) {
        pacman.dir = LEFT;
        pacman.moving = true;
    } else if (game.right && !game.left) {
        pacman.dir = RIGHT;
        pacman.moving = true;
    }

    if (game.up && !game.down) {
        pacman.dir = UP;
        pacman.moving = true;
    } else if (game.down && !game.up) {
        pacman.dir = DOWN;
        pacman.moving = true;
    }

    if (!canmove(&pacman)) {
        pacman.moving = false;
        return;
    }

    if (!pacman.moving) return;

    if (pacman.dir == LEFT)
        pacman.pos.x -= pacman.speed;
    else if (pacman.dir == RIGHT)
        pacman.pos.x += pacman.speed;
    else if (pacman.dir == UP)
        pacman.pos.y -= pacman.speed;
    else if (pacman.dir == DOWN)
        pacman.pos.y += pacman.speed;
    else
        assert(false && "unknown direction in update_2pac");


    /* only enter a new tile when your center has crossed the center of the new tile */
    v2f_t cur_tile_pos = { 
        .x = (pacman.tile % BOARD_WIDTH) + 0.5f*TILE_SIZE, 
        .y = (pacman.tile / BOARD_WIDTH) + 0.5f*TILE_SIZE };

    if (vec2f_dist(cur_tile_pos, pacman.pos) >= TILE_SIZE) {
        int tile_x = (int)(pacman.pos.x) / TILE_SIZE;
        int tile_y = (int)(pacman.pos.y) / TILE_SIZE;
        int next_tile = tile_y * BOARD_WIDTH + tile_x;
        pacman.tile = next_tile;
    }
}
