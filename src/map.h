#ifndef MAP_H
#define MAP_H

#include "portal.h"
#include <SDL2/SDL.h>

struct Map
{
    char *layout;
    SDL_Point size;
    int tile_size;

    struct Portal *portal_1, *portal_2;
};

struct Map *map_init(const char *path, SDL_Point size, int tile_size);
void map_cleanup(struct Map *self);

SDL_FPoint map_get_random_empty_spot(struct Map *self);

struct Portal *map_check_portal_collision(struct Map *self, SDL_Point grid_pos, int dir);

#endif

