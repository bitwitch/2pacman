#include <assert.h>
#include <stdio.h>
#include "globals.h"
#include "vecs.h"
#include "stb_ds.h"

#define GHOST_HOUSE_EXIT_TILE ((112/TILE_SIZE) * BOARD_WIDTH + (104/TILE_SIZE))


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

/* forbid moving up through the four specific tiles near the center of the
 * board, these are just hardcoded:
 *   row 13, col 12 -> 376
 *   row 13, col 15 -> 379
 *   row 25, col 12 -> 712
 *   row 15, col 15 -> 715
 */
static bool up_forbidden(int tile) {
    return tile == 376 || tile == 379 || tile == 712 || tile == 715;
}

static void available_directions(ghost_t *e, bool options[4]) {
    int tile;
    for (int dir=0; dir<4; ++dir) {
        tile = get_adjacent_tile(e->tile, dir);
        options[dir] = !is_solid(board[tile]);
        if (board[tile] == 'n' && dir == DOWN)
            options[DOWN] = false;
        if (dir == UP && up_forbidden(tile))
            options[UP] = false;
    }

    /* only allow left and right movement through the tunnel */
    if (e->pos.x < TILE_SIZE || e->pos.x > (BOARD_WIDTH-1)*TILE_SIZE) {
        if (e->dir == LEFT)  options[LEFT] = true;
        else if (e->dir == RIGHT) options[RIGHT] = true;
        options[UP] = false;
        options[DOWN] = false;
    }

    /* don't allow reversing direction */
    if (e->dir == LEFT)       options[RIGHT] = false;
    else if (e->dir == RIGHT) options[LEFT] = false;
    else if (e->dir == UP)    options[DOWN] = false;
    else if (e->dir == DOWN)  options[UP] = false;
    else
        assert(false && "unknown direction in available_directions");

}

void move_towards_target(ghost_t *e) {
    bool options[4]; 
    available_directions(e, options);

    /* TODO(shaw): decrease ghost speed if travelling through tunnel */

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

static void ghost_hover(ghost_t *g) {
    if (g->dir == UP)
        g->pos.y -= g->speed;
    else if (g->dir == DOWN)
        g->pos.y += g->speed;

    /* only enter a new tile when your center has crossed the center of the new tile */
    v2f_t cur_tile_pos = get_tile_pos(g->tile);
    if (vec2f_dist(cur_tile_pos, g->pos) >= TILE_SIZE) {
        g->tile = tile_at(g->pos);

        int tile = get_adjacent_tile(g->tile, g->dir);
        if (board[tile] != ' ')
            g->dir = g->dir == UP ? DOWN : UP;
    }
}


static void update_single_ghost(ghost_t *g) {
    g->anim_timer -= TIME_STEP;
    if (g->anim_timer <= 0) {
        g->frame = g->frame ? 0 : 1;
        g->anim_timer = g->anim_frame_time;
    }

    switch (g->state) {
        case HOUSE_PARTY:
            ghost_hover(g);
            if (g->ghost_house_timer - TIME_STEP < 0) {
                g->ghost_house_timer = 0;
                g->state = EXIT_HOUSE;
                g->target_tile = GHOST_HOUSE_EXIT_TILE;
                printf("EXITING HOUSE\n");
            } else {
                g->ghost_house_timer -= TIME_STEP;
            }
            break;

        case EXIT_HOUSE:
            move_towards_target(g);
            if (g->tile == GHOST_HOUSE_EXIT_TILE) {
                g->dir = LEFT;
                g->state = NORMAL;
                if (game.ghostmode == SCATTER)
                    g->target_tile = g->scatter_target_tile;
                else if (game.ghostmode == CHASE)
                    /* NOTE(shaw): only really need to set one target here, but probably doesn't matter */
                    set_chase_targets();
            }
            break;

        case NORMAL:
            move_towards_target(g);
            break;
    }
}

void update_ghosts(void) {
    if (game.ghostmode == CHASE)
        set_chase_targets();

    for (int i=0; i<GHOST_COUNT; ++i)
        update_single_ghost(&ghosts[i]);
}

SDL_Rect ghost_animation_frame(ghost_t *ghost) {
    SDL_Rect rect;

    /* flee mode */
    if (game.ghostmode == FLEE) {
        rect = hmget(tilemap, ghosts[BLINKY].c);
        rect.x += 8*ghosts[BLINKY].w;
        if (game.flee_timer <= 2*SEC_TO_USEC) {
            int blink = game.flee_timer / (int64_t)125000;
            if (blink % 2 == 0)
                rect.x += 2*ghosts[BLINKY].w;
        }

    /* get sprite for each direction */
    } else {
        rect = hmget(tilemap, ghost->c);
        if (ghost->dir == LEFT)
            rect.x += 2*ghost->w;
        else if (ghost->dir == UP)
            rect.x += 4*ghost->w;
        else if (ghost->dir == DOWN)
            rect.x += 6*ghost->w;
    }

    /* get the correct frame for this animation */
    if (ghost->frame == 1)
        rect.x += ghost->w;
    
    return rect;
}




