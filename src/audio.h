#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <SDL2/SDL_mixer.h>

void audio_init();
void audio_cleanup();

void audio_play_music(const char *path);
void audio_stop_music();

void audio_play_sound(const char *path);
void audio_stop_finished_sounds();

void audio_mute(bool sound, bool music);

#endif

