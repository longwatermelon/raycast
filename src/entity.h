#ifndef ENTITY_H
#define ENTITY_H

#include "map.h"
#include <SDL.h>

struct Player;

struct Entity
{
    SDL_FPoint pos;

    SDL_Texture* sprite;
    SDL_Point sprite_size;
};

struct Entity* entity_init(SDL_FPoint pos, SDL_Renderer* rend, const char* sprite_path);
void entity_cleanup(struct Entity* e);

void entity_move(struct Entity* e, float x, float y);
void entity_move_towards_player(struct Entity* e, struct Player* p, struct Map* map);

#endif

