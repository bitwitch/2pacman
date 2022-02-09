#include <assert.h>
#include <stdio.h>
#include "globals.h"
#include "vecs.h"

void set_scatter_targets(void) {
    for (int i=0; i<GHOST_COUNT ; ++i)
        ghosts[i].target_tile = ghosts[i].scatter_target_tile;
}

void available_directions(entity_t *e, bool options[4]) {
    int tile;
    for (int dir=0; dir<4; ++dir) {
        tile = get_adjacent_tile(e->tile, dir);
        options[dir] = !is_solid(board[tile]);
    }

    if (e->dir == LEFT)       options[RIGHT] = false;
    else if (e->dir == RIGHT) options[LEFT] = false;
    else if (e->dir == UP)    options[DOWN] = false;
    else if (e->dir == DOWN)  options[UP] = false;
    else
        assert(false && "unknown direction in available_directions");
}

void move_towards_target(entity_t *e) {
    if (e->dir == LEFT)
        e->pos.x -= e->speed;
    else if (e->dir == RIGHT)
        e->pos.x += e->speed;
    else if (e->dir == UP)
        e->pos.y -= e->speed;
    else if (e->dir == DOWN)
        e->pos.y += e->speed;
    else
        assert(false && "unknown direction in move_towards_target");

    /* only enter a new tile when your center has crossed the center of the new tile */
    v2f_t cur_tile_pos = get_tile_pos(e->tile);
    if (vec2f_dist(cur_tile_pos, e->pos) >= TILE_SIZE) {
        int tile_x = (int)(e->pos.x) / TILE_SIZE;
        int tile_y = (int)(e->pos.y) / TILE_SIZE;
        int next_tile = tile_y * BOARD_WIDTH + tile_x;
        e->tile = next_tile;

        /* just entered a new tile, make a decision on next direction to pick */

        /*if current tile has more than one open neighbor, take the one that is closest to target*/
        /*cannot move back to previous tile*/
        bool options[4]; 
        available_directions(e, options);

        float shortest = BOARD_HEIGHT*TILE_SIZE*69.0f; /* arbitrary sufficiently large number */
        for (int dir=0; dir<4; ++dir) {
            if (!options[dir]) continue;

            int tile = get_adjacent_tile(e->tile, dir);

            float dist = dist_tiles(tile, e->target_tile);
            if (dist < shortest) {
                shortest = dist;
                e->dir = dir;
            }
        }
    }
}

static void ghost_hover(entity_t *e) {
    if (e->dir == UP)
        e->pos.y -= e->speed;
    else if (e->dir == DOWN)
        e->pos.y += e->speed;

    /* only enter a new tile when your center has crossed the center of the new tile */
    v2f_t cur_tile_pos = get_tile_pos(e->tile);
    if (vec2f_dist(cur_tile_pos, e->pos) >= TILE_SIZE) {
        int tile_x = (int)(e->pos.x) / TILE_SIZE;
        int tile_y = (int)(e->pos.y) / TILE_SIZE;
        int next_tile = tile_y * BOARD_WIDTH + tile_x;
        e->tile = next_tile;

        int tile = get_adjacent_tile(e->tile, e->dir);
        if (board[tile] != ' ')
            e->dir = e->dir == UP ? DOWN : UP;
    }
}

#define GHOST_HOUSE_EXIT_TILE ((88/TILE_SIZE) * BOARD_WIDTH + (104/TILE_SIZE))

static void update_single_ghost(entity_t *e) {
    switch (e->state) {
        case HOUSE_PARTY:
            ghost_hover(e);
            if (e->ghost_house_timer - (uint64_t)TIME_STEP > e->ghost_house_timer) {
                e->ghost_house_timer = 0;
                e->state = EXIT_HOUSE;
                e->target_tile = GHOST_HOUSE_EXIT_TILE;
                printf("EXITING HOUSE\n");
            } else {
                e->ghost_house_timer -= (uint64_t)TIME_STEP;
            }
            break;

        case EXIT_HOUSE:
            move_towards_target(e);
            if (e->tile == GHOST_HOUSE_EXIT_TILE) {
                e->dir = LEFT;
                e->state = NORMAL;
                if (game.ghostmode == SCATTER)
                    e->target_tile = e->scatter_target_tile;
                else
                    e->target_tile = e->scatter_target_tile;

            }
            break;

        case NORMAL:
            move_towards_target(e);
            break;
    }
}

void update_ghosts(void) {
    for (int i=0; i<GHOST_COUNT; ++i)
        update_single_ghost(&ghosts[i]);
}




