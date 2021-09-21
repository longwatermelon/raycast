#ifndef MAP_H
#define MAP_H

#include <SDL.h>

struct Map
{
    char* layout;
    SDL_Point size;
    int tile_size;
};

struct Map* map_init(const char* path, SDL_Point size, int tile_size);
void map_cleanup(struct Map* m);

#endif

