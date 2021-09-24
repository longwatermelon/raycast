#ifndef ENTITY_H
#define ENTITY_H

#include <SDL.h>

struct Entity
{
    SDL_Point pos;
    SDL_Texture* sprite;
};

struct Entity* entity_init(SDL_Point pos, SDL_Renderer* rend, const char* sprite_path);
void entity_cleanup(struct Entity* e);

#endif

