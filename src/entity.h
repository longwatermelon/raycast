#ifndef ENTITY_H
#define ENTITY_H

#include "map.h"
#include <stdbool.h>
#include <SDL2/SDL.h>

struct Player;

struct Entity
{
    enum
    {
        ENTITY_ENEMY,
        ENTITY_AMMO,
        ENTITY_GRENADE,
        ENTITY_NUTS
    } type;

    SDL_FPoint pos;
    float speed;

    SDL_Texture *sprite;
    SDL_Point sprite_size;

    int width;

    // Entity type specific properties
    bool enemy_dead;
    struct timespec enemy_death_time;

    bool blasted;
    SDL_FPoint blast_dir;
};

struct Entity *entity_init(int type, SDL_FPoint pos, SDL_Renderer *rend, const char *sprite_path);
void entity_cleanup(struct Entity *self);

void entity_move(struct Entity *self, SDL_Renderer *rend, struct Map *map, float x, float y);
void entity_move_towards_player(struct Entity *self, SDL_Renderer *rend, struct Player *p, struct Map *map);

void entity_die(struct Entity *self, SDL_Renderer *rend, int weapon);

#endif

