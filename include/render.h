#ifndef RENDER_H
#define RENDER_H

void render_menu(void);
void render_game(char *board, int board_size);
void render_level_completed(void);
SDL_Texture *load_texture(char *filename);

#endif
