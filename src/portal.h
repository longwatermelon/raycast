#ifndef PORTAL_H
#define PORTAL_H

#include <stdbool.h>
#include <SDL2/SDL.h>

struct Portal
{
    SDL_Point grid_pos;
    int dir;
};

struct Portal *portal_alloc(SDL_Point grid_pos, int dir);
void portal_free(struct Portal *p);

bool portal_check_collision(struct Portal *p, SDL_Point grid_pos, int dir);

#endif

