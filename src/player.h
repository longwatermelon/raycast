#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"
#include "entity.h"
#include <stdbool.h>
#include <SDL.h>

enum
{
    COLLISION_HORIZONTAL,
    COLLISION_VERTICAL
};

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

    bool alive;

    bool shooting;
    clock_t last_shot_time;
    bool reloading;
    int bullets;
    int bullets_loaded;
};

struct Player* player_init(SDL_Point pos, float angle);
void player_cleanup(struct Player* p);

void player_render(struct Player* p, SDL_Renderer* rend, struct Map* map, struct Entity** entities, size_t entities_size);

void player_move(struct Player* p, struct Map* map);

// Returns end point of casted ray
SDL_Point player_cast_ray(struct Player* p, float angle, struct Map* map, struct Entity** entities, size_t entities_size, int* collision_type);
SDL_Point player_cast_ray_horizontal(struct Player* p, float angle, struct Map* map);
SDL_Point player_cast_ray_vertical(struct Player* p, float angle, struct Map* map);

// Returns ray length of ray that intersects with entity
// If target type is -1, the ray will apply to all entity types
int player_cast_ray_entity(struct Player* p, float angle, struct Entity** entities, size_t entities_size, struct Entity** ignored_entities, size_t ignored_entities_size, int target_type, float* intersection, struct Entity** entity_hit);

#endif

