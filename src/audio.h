#ifndef AUDIO_H
#define AUDIO_H

#include <SDL_mixer.h>

void audio_init();
void audio_cleanup();

void audio_play_music(const char* path);
void audio_stop_music();

void audio_play_sound(const char* path);
void audio_stop_finished_sounds();

#endif

