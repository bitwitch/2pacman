#include <assert.h>
#include <stdio.h>
#include "globals.h"
#include "sound.h"
#include "vecs.h"
#include "stb_ds.h"

static bool forward_collide() {
    /* ignore collisions in tunnel, movement is restricted to left right anyway */
    if (pacman.pos.x < TILE_SIZE || pacman.pos.x > (BOARD_WIDTH-1)*TILE_SIZE)
        return false;

    /*check tile one ahead in travel direction*/
    int front_tile = get_adjacent_tile(pacman.tile, pacman.dir);

    /* ghost house door */
    if (board[front_tile] == 'n') return true;
 
    if (!is_solid(board[front_tile])) {
        return false;
    } else {
        v2f_t front_tile_pos = get_tile_pos(front_tile);
        if (vec2f_dist(front_tile_pos, pacman.pos) > TILE_SIZE)
            return false;
    }

    return true;
}

static void available_directions(bool options[4]) {
    int tile;
    for (int dir=0; dir<4; ++dir) {
        tile = get_adjacent_tile(pacman.tile, dir);
        options[dir] = !is_solid(board[tile]);
        if (board[tile] == 'n')
            options[dir] = false;
    }

    /* only allow left and right movement through the tunnel */
    if (pacman.pos.x < TILE_SIZE || pacman.pos.x > (BOARD_WIDTH-1)*TILE_SIZE) {
        if (pacman.dir == LEFT)  options[LEFT] = true;
        else if (pacman.dir == RIGHT) options[RIGHT] = true;
        options[UP] = false;
        options[DOWN] = false;
    }

}

static void update_dead_2pac(void) {
    static int death_channel;
    if (!pacman.show) {
        game.scene_timer -= TIME_STEP;
        if (game.scene_timer < 0)
            restart_from_death();
    } else {
        /* update animation frame */
        pacman.anim_timer -= 0.5*TIME_STEP;
        if (pacman.anim_timer <= 0) {

            if (pacman.frame == 0)
                death_channel = Mix_PlayChannel(-1, game.samples[DEATH_1], 0);
            if (pacman.frame == 10) {
                Mix_HaltChannel(death_channel);
                Mix_PlayChannel(death_channel, game.samples[DEATH_2], 1);
            }

            if (++pacman.frame > 10) {
                if (pacman.lives == 0) {
                    game.mode = GAME_OVER;
                    game.scene_timer = game.game_over_duration;
                    hud_items[GAME_OVER_LABEL].show = true;
                    pacman.show = false;
                } else {
                    game.scene_timer = 1*SEC_TO_USEC;
                    pacman.show = false;
                } 
            }
            pacman.anim_timer = pacman.anim_frame_time;
        }
    }
}

void update_2pac(void) {
    if (pacman.dead) {
        pacman.death_timer -= TIME_STEP;
        if (pacman.death_timer < 0)
            update_dead_2pac();
        return;
    } else if (game.ghost_eaten_timer > 0) {
        return;
    }

    if (pacman.delay_frames > 0) {
        --pacman.delay_frames;
        return;
    }

    bool options[4];
    available_directions(options);

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

    /* update animation frame */
    pacman.anim_timer -= TIME_STEP;
    if (pacman.anim_timer <= 0) {
        if (++pacman.frame > 2)
            pacman.frame = 0;
        pacman.anim_timer = pacman.anim_frame_time;
    }

    /* calculate speed */
    float mult;
    int index = game.level-1;
    if (index == 0)
        mult = game.ghostmode == FLEE ? .9 : .8;
    else if (index < 4)
        mult = game.ghostmode == FLEE ? .95 : .9;
    else if (index < 20)
        mult = 1.0;
    else 
        mult = .9;

    float speed = game.full_speed * mult;

    /* handle movement through the tunnel */
    if (pacman.pos.x < TILE_SIZE || pacman.pos.x > (BOARD_WIDTH-1)*TILE_SIZE) {
        if (pacman.dir == LEFT) { 
            pacman.pos.x -= speed;
            pacman.moving = true;
        }
        else if (pacman.dir == RIGHT) {
            pacman.pos.x += speed;
            pacman.moving = true;
        }

        if (pacman.pos.x < -TILE_SIZE && pacman.dir == LEFT)
            pacman.pos.x = BOARD_WIDTH*TILE_SIZE + TILE_SIZE;

        if (pacman.pos.x > BOARD_WIDTH*TILE_SIZE + TILE_SIZE && pacman.dir == RIGHT)
            pacman.pos.x = -TILE_SIZE;

        return;
    }

    /* move towards target tile */
    pacman.target_tile = get_adjacent_tile(pacman.tile, pacman.dir);
    v2f_t target_tile_pos = get_tile_pos(pacman.target_tile);
    v2f_t dir_vec = vec2f_norm(vec2f_sub(target_tile_pos, pacman.pos));
    pacman.pos = vec2f_add(pacman.pos, vec2f_scale(dir_vec, speed));

    /* only enter a new tile when your center has crossed the center of the new tile */
    v2f_t cur_tile_pos = get_tile_pos(pacman.tile);

    if (vec2f_dist(cur_tile_pos, pacman.pos) >= TILE_SIZE) {
        int tile_x = (int)(pacman.pos.x + 0.5f) / TILE_SIZE;
        int tile_y = (int)(pacman.pos.y + 0.5f) / TILE_SIZE;
        int next_tile = tile_y * BOARD_WIDTH + tile_x;
        pacman.tile = next_tile;
    }
}

SDL_Rect pacman_animation_frame() {
    SDL_Rect rect = hmget(tilemap, pacman.c);

    if (pacman.dead && pacman.death_timer < 0) {
        rect.x += (3+pacman.frame)*pacman.w;
        return rect;
    }

    if (pacman.frame == 0) {
        rect.x += 2*pacman.w;
        return rect;
    }

    if (pacman.dir == LEFT)
        rect.y += 1*pacman.w;
    else if (pacman.dir == UP)
        rect.y += 2*pacman.w;
    else if (pacman.dir == DOWN)
        rect.y += 3*pacman.w;

    if (pacman.frame == 2)
        rect.x += pacman.w;

    return rect;
}


