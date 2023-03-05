#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>

struct Map
{
    char *layout;
    SDL_Point size;
    int tile_size;
};

struct Map *map_init(const char *path, SDL_Point size, int tile_size);
void map_cleanup(struct Map *self);

SDL_FPoint map_get_random_empty_spot(struct Map *self);

void map_damage_wall(struct Map *self, SDL_Point gpos);

#endif

