#ifndef GHOSTS_H
#define GHOSTS_H

void update_ghosts(void);
void set_scatter_targets(void);
void reverse_ghosts(void);
void frighten_ghosts(bool value);
SDL_Rect ghost_animation_frame(ghost_t *ghost);

#endif
