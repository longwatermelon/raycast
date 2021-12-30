#ifndef COMMON_H
#define COMMON_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

char *common_read_file(const char *path);
float common_restrict_angle(float angle);

SDL_Texture *common_render_text(SDL_Renderer *rend, TTF_Font *font, const char *text);
void common_display_statistic(SDL_Renderer *rend, TTF_Font *font, const char *text, int value, SDL_Point pos);

float common_time_diff(struct timespec t1, struct timespec t2);

#endif

