#ifndef RENDER_H
#define RENDER_H

void render_menu(float interp);
void render_game(char *board, int board_size, float interp);
SDL_Texture *load_texture(char *filename);

#endif
