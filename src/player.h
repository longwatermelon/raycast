#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"
#include <stdbool.h>
#include <SDL.h>

struct Player
{
    SDL_FRect rect;
    float speed;

    float angle;
    float angle_change;

    enum
    {
        RAY_HORIZONTAL,
        RAY_VERTICAL,
        RAY_ALL
    } ray_mode;
};

struct Player* player_init(SDL_Point pos, float angle);
void player_cleanup(struct Player* p);

void player_render(struct Player* p, SDL_Renderer* rend, struct Map* map);

void player_move(struct Player* p, struct Map* map);

// Returns end point of casted ray
SDL_Point player_cast_ray(struct Player* p, float angle, struct Map* map, bool* horizontal);
SDL_Point player_cast_ray_horizontal(struct Player* p, float angle, struct Map* map);
SDL_Point player_cast_ray_vertical(struct Player* p, float angle, struct Map* map);

#endif

