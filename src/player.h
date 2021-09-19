#ifndef PLAYER_H
#define PLAYER_H

#include <SDL.h>

struct Player
{
    SDL_FRect rect;
    float speed;

    float angle;
    float angle_change;
};

struct Player* player_init(SDL_Point pos, float angle);
void player_cleanup(struct Player* p);

void player_render(struct Player* p, SDL_Renderer* rend, char* map, int map_width, int tile_size);

void player_move(struct Player* p, char* map, int map_width, int tile_size);

// Returns end point of casted ray
SDL_Point player_cast_ray(struct Player* p, float angle, char* map, int map_width, int tile_size);
SDL_Point player_cast_ray_horizontal(struct Player* p, float angle, char* map, int map_width, int tile_size);
SDL_Point player_cast_ray_vertical(struct Player* p, float angle, char* map, int map_width, int tile_size);

#endif

