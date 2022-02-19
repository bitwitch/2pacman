#include "globals.h"
#include "sound.h"

static const char* wav_filenames[SOUNDS_COUNT] = {
    "assets/munch_1.wav",      /* MUNCH_1 */
    "assets/munch_2.wav",      /* MUNCH_2 */
    "assets/eat_ghost.wav",    /* EAT_GHOST */
    "assets/death_1.wav",      /* DEATH_1 */
    "assets/death_2.wav",      /* DEATH_2 */
    "assets/game_start.wav",   /* GAME_START */
    "assets/retreating.wav",   /* RETREAT */
    "assets/eat_fruit.wav",    /* EAT_FRUIT */
    "assets/flee_mode.wav",    /* FLEE_MUSIC */
    "assets/pacman_music.ogg", /* MAIN_MUSIC */
};

void init_sound(void) {
    // Set up the audio stream
    int result = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
    if (result < 0) {
        fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
        exit(1);
    } 

    result = Mix_AllocateChannels(4);
    if (result < 0) {
        fprintf(stderr, "Unable to allocate mixing channels: %s\n", SDL_GetError());
        exit(-1);
    }

    // Load waveforms
    for (int i=0; i<SOUNDS_COUNT; ++i) {
        printf("[INFO] Loading sound %s\n", wav_filenames[i]);
        game.samples[i] = Mix_LoadWAV(wav_filenames[i]);
        if (game.samples[i] == NULL )
            fprintf(stderr, "Unable to load wave file: %s\n", wav_filenames[i]);
    }
}
