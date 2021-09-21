#ifndef ENTITY_H
#define ENTITY_H

#include <SDL.h>

struct Entity
{
    SDL_Point pos;
};

struct Entity* entity_init(SDL_Point pos);
void entity_cleanup(struct Entity* e);

#endif

