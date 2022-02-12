#include <assert.h>
#include <stdio.h>
#include "globals.h"
#include "vecs.h"

void set_scatter_targets(void) {
    for (int i=0; i<GHOST_COUNT; ++i)
        ghosts[i].target_tile = ghosts[i].scatter_target_tile;
}


/* TODO(shaw): investigate using padding on all sides of the board, and using a
 * stride, this seems like it might make some things easier and cleaner. For
 * example calculating chase targets 
 */

static void set_chase_targets(void) {
    /* BLINKY */
    if (ghosts[BLINKY].state != EXIT_HOUSE) {
        ghosts[BLINKY].target_tile = pacman.tile;
    }

    /* PINKY */
    if (ghosts[PINKY].state != EXIT_HOUSE) {
        int pacman_row = pacman.tile / BOARD_WIDTH;
        int target_tile;
        switch (pacman.dir) {
            case RIGHT: 
                target_tile = pacman.tile + 4;
                /* handle if target wraps the board */
                if (target_tile/BOARD_WIDTH != pacman_row)
                    target_tile = pacman_row*BOARD_WIDTH + BOARD_WIDTH-1;
                break;
            case DOWN: 
                target_tile = pacman.tile + 4*BOARD_WIDTH;
                break;
            case LEFT: 
                target_tile = pacman.tile - 4;
                /* handle if target wraps the board */
                if (target_tile/BOARD_WIDTH != pacman_row)
                    target_tile = pacman_row*BOARD_WIDTH;
                break;
            case UP: 
                target_tile = pacman.tile - 4*BOARD_WIDTH - 4; 
                /* TODO(shaw): handle if target wraps the board on the left
                 * because of the left offset */
                break;
            default: 
                assert(false && "unknown dir in set_chase_targets");
                break;
        }
        ghosts[PINKY].target_tile = target_tile;
    }

    /* INKY */
    if (ghosts[INKY].state != EXIT_HOUSE) {
        v2f_t ahead_vec = {0};
        switch (pacman.dir) {
            case RIGHT: ahead_vec.x =  2*TILE_SIZE; break;
            case  LEFT: ahead_vec.x = -2*TILE_SIZE; break;
            case  DOWN: ahead_vec.y =  2*TILE_SIZE; break;
            case    UP: ahead_vec.y = -2*TILE_SIZE; break;
            default: 
                assert(false && "unknown dir in set_chase_targets");
                break;
        }

        v2f_t ahead_tile_pos = vec2f_add(get_tile_pos(pacman.tile), ahead_vec);
        v2f_t blinky_pos = get_tile_pos(ghosts[BLINKY].tile);

        v2f_t blinky_to_ahead = vec2f_sub(ahead_tile_pos, blinky_pos);
        v2f_t target_pos = vec2f_add(blinky_pos, vec2f_scale(blinky_to_ahead, 2));

        if (target_pos.x > BOARD_WIDTH*TILE_SIZE) {
            v2f_t right = {1, 0};
            float cos_angle = vec2f_dot(right, vec2f_norm(blinky_to_ahead));
            float scale = (BOARD_WIDTH*TILE_SIZE - 0.5*TILE_SIZE - blinky_pos.x) / cos_angle;
            blinky_to_ahead = vec2f_scale(vec2f_norm(blinky_to_ahead), scale);
            target_pos = vec2f_add(blinky_pos, blinky_to_ahead);
        }

        ghosts[INKY].target_tile = tile_at(target_pos);
        /* TODO(shaw): need to handle if the target tile wraps the board */
    }

    /* CLYDE */
    if (ghosts[CLYDE].state != EXIT_HOUSE) {
        if (vec2f_dist(ghosts[CLYDE].pos, pacman.pos) > 8*TILE_SIZE)
            ghosts[CLYDE].target_tile = pacman.tile;
        else
            ghosts[CLYDE].target_tile = ghosts[CLYDE].scatter_target_tile;
    }

}


void reverse_ghosts(void) {
    for (int i=0; i<GHOST_COUNT; ++i)
        ghosts[i].reverse = true;
}


static void available_directions(entity_t *e, bool options[4]) {
    int tile;
    for (int dir=0; dir<4; ++dir) {
        tile = get_adjacent_tile(e->tile, dir);
        options[dir] = !is_solid(board[tile]);
        if (board[tile] == 'n' && dir == DOWN)
            options[DOWN] = false;
    }

    /* only allow left and right movement through the tunnel */
    if (e->pos.x < TILE_SIZE || e->pos.x > (BOARD_WIDTH-1)*TILE_SIZE) {
        if (e->dir == LEFT)  options[LEFT] = true;
        else if (e->dir == RIGHT) options[RIGHT] = true;
        options[UP] = false;
        options[DOWN] = false;
    }

    if (e->dir == LEFT)       options[RIGHT] = false;
    else if (e->dir == RIGHT) options[LEFT] = false;
    else if (e->dir == UP)    options[DOWN] = false;
    else if (e->dir == DOWN)  options[UP] = false;
    else
        assert(false && "unknown direction in available_directions");

}

void move_towards_target(entity_t *e) {
    bool options[4]; 
    available_directions(e, options);

    if (e->dir == LEFT && options[LEFT])
        e->pos.x -= e->speed;
    else if (e->dir == RIGHT && options[RIGHT])
        e->pos.x += e->speed;
    else if (e->dir == UP && options[UP])
        e->pos.y -= e->speed;
    else if (e->dir == DOWN && options[DOWN])
        e->pos.y += e->speed;

    if (e->pos.x < 0) {
        if (e->pos.x < -TILE_SIZE && e->dir == LEFT)
            e->pos.x = BOARD_WIDTH*TILE_SIZE + TILE_SIZE;
        return;
    }

    if (e->pos.x > BOARD_WIDTH*TILE_SIZE) {
        if (e->pos.x > BOARD_WIDTH*TILE_SIZE + TILE_SIZE && e->dir == RIGHT)
            e->pos.x = -TILE_SIZE;
        return;
    }



    /* only enter a new tile when your center has crossed the center of the new tile */
    v2f_t cur_tile_pos = get_tile_pos(e->tile);
    if (vec2f_dist(cur_tile_pos, e->pos) >= TILE_SIZE) {
        e->tile = tile_at(e->pos);

        if (e->reverse && e->state == NORMAL) {
            switch (e->dir) {
                case    UP: e->dir = DOWN;  break;
                case  DOWN: e->dir = UP;    break;
                case  LEFT: e->dir = RIGHT; break;
                case RIGHT: e->dir = LEFT;  break;
                default: break;
            }
            e->reverse = false;
        }

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
        e->tile = tile_at(e->pos);

        int tile = get_adjacent_tile(e->tile, e->dir);
        if (board[tile] != ' ')
            e->dir = e->dir == UP ? DOWN : UP;
    }
}

#define GHOST_HOUSE_EXIT_TILE ((112/TILE_SIZE) * BOARD_WIDTH + (104/TILE_SIZE))

static void update_single_ghost(entity_t *e) {
    switch (e->state) {
        case HOUSE_PARTY:
            ghost_hover(e);
            if (e->ghost_house_timer - TIME_STEP < 0) {
                e->ghost_house_timer = 0;
                e->state = EXIT_HOUSE;
                e->target_tile = GHOST_HOUSE_EXIT_TILE;
                printf("EXITING HOUSE\n");
            } else {
                e->ghost_house_timer -= TIME_STEP;
            }
            break;

        case EXIT_HOUSE:
            move_towards_target(e);
            if (e->tile == GHOST_HOUSE_EXIT_TILE) {
                e->dir = LEFT;
                e->state = NORMAL;
                if (game.ghostmode == SCATTER)
                    e->target_tile = e->scatter_target_tile;
                else if (game.ghostmode == CHASE)
                    /* NOTE(shaw): only really need to set one target here, but probably doesn't matter */
                    set_chase_targets();

            }
            break;

        case NORMAL:
            move_towards_target(e);
            break;
    }
}

void update_ghosts(void) {
    if (game.ghostmode == CHASE)
        set_chase_targets();

    for (int i=0; i<GHOST_COUNT; ++i)
        update_single_ghost(&ghosts[i]);
}




