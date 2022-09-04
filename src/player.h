#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"
#include "entity.h"
#include <stdbool.h>
#include <SDL2/SDL.h>

struct Prog;

enum
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

struct PlayerModeData
{
    enum
    {
        PLAYER_MODE_NORMAL,
        PLAYER_MODE_GRAPPLING
    } mode;

    SDL_Point grappling_dst;
    float grappling_theta;
    float grappling_drot;
};

struct PlayerAnimationData
{
    bool gun_at_bottom;
    SDL_Rect gun_pos;
    SDL_Point gun_default_pos;

    bool knife_outstretched;
    SDL_Rect knife_pos;
    SDL_Point knife_default_pos;

    int switching_weapon;
};

struct Player
{
    SDL_FPoint pos;

    float angle;
    float angle_change;

    enum
    {
        RAY_HORIZONTAL,
        RAY_VERTICAL,
        RAY_ALL
    } ray_mode;

    int health;
    struct timespec last_hurt_time;

    // Gun properties
    bool shooting;
    struct timespec last_shot_time;
    bool reloading;
    int bullets;
    int bullets_loaded;

    // Knife properties
    bool swinging;

    int enemies_killed;

    struct PlayerModeData mode_data;

    enum
    {
        WEAPON_GUN,
        WEAPON_KNIFE
    } weapon;

    struct PlayerAnimationData animation;

    SDL_Texture *gun_texture;
    SDL_Texture *shot_texture;
    SDL_Texture *knife_texture;

    bool detect_collisions;
    bool ignore_walls_when_shooting;
};

struct Player *player_init(SDL_Point pos, float angle, SDL_Renderer *rend);
void player_cleanup(struct Player *self);

void player_render_weapon(struct Player *self, SDL_Renderer *rend);
void player_advance_animations(struct Player *self);

void player_move(struct Player *self, struct Map *map, float x, float y);

void player_execute_mode(struct Player *self);

// Returns entity attacked
struct Entity *player_attack(struct Player *self, struct Entity **entities, size_t entities_size, struct Map *map);

// Returns end point of casted ray
SDL_Point player_cast_ray(struct Player *self, float angle, struct Map *map, struct Entity **entities, size_t entities_size, int *collision_type);
SDL_Point player_cast_ray_horizontal(struct Player *self, float angle, struct Map *map);
SDL_Point player_cast_ray_vertical(struct Player *self, float angle, struct Map *map);

// Returns ray length of ray that intersects with entity
// If target type is -1, the ray will apply to all entity types
int player_cast_ray_entity(struct Player *self, float angle, struct Entity **entities, size_t entities_size, struct Entity **ignored_entities, size_t ignored_entities_size, int target_type, float *intersection, struct Entity **entity_hit);

struct Entity *player_shoot(struct Player *self, struct Entity **entities, size_t entities_size, struct Map *map);
struct Entity *player_slash(struct Player *self, struct Entity **entities, size_t entities_size, struct Map *map);

#endif

