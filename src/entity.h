#ifndef ENTITY_H
#define ENTITY_H

#include "map.h"
#include <SDL.h>

struct Player;

struct Entity
{
    enum
    {
        ENTITY_ENEMY,
        ENTITY_AMMO
    } type;

    SDL_FPoint pos;
    float speed;

    SDL_Texture* sprite;
    SDL_Point sprite_size;

    int width;
};

struct Entity* entity_init(int type, SDL_FPoint pos, SDL_Renderer* rend, const char* sprite_path);
void entity_cleanup(struct Entity* e);

void entity_move(struct Entity* e, struct Map* map, float x, float y);
void entity_move_towards_player(struct Entity* e, struct Player* p, struct Map* map);

#endif

