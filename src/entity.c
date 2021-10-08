#include "entity.h"
#include "player.h"
#include "common.h"
#include <time.h>
#include <SDL_image.h>


struct Entity* entity_init(int type, SDL_FPoint pos, SDL_Renderer* rend, const char* sprite_path)
{
    struct Entity* self = malloc(sizeof(struct Entity));
    self->type = type;

    self->pos = pos;
    self->speed = fmod(rand(), 4.f) + 2.f;

    self->sprite = IMG_LoadTexture(rend, sprite_path);
    SDL_QueryTexture(self->sprite, 0, 0, &self->sprite_size.x, &self->sprite_size.y);

    self->width = 20.f;

    self->enemy_dead = false;

    return self;
}


void entity_cleanup(struct Entity* self)
{
    SDL_DestroyTexture(self->sprite);
    free(self);
}


void entity_move(struct Entity* self, struct Map* map, float x, float y)
{
    int xo = 0;

    if (x > 0)
        xo = 10;
    if (x < 0)
        xo = -10;

    int yo = 0;

    if (y > 0)
        yo = 10;
    if (y < 0)
        yo = -10;

    SDL_Point grid_pos = {
        (int)(self->pos.x - ((int)(self->pos.x) % map->tile_size)) / map->tile_size,
        (int)(self->pos.y - ((int)(self->pos.y) % map->tile_size)) / map->tile_size
    };

    SDL_Point new_grid_pos = {
        (int)((self->pos.x + x + xo) - ((int)(self->pos.x + x + xo) % map->tile_size)) / map->tile_size,
        (int)((self->pos.y + y + yo) - ((int)(self->pos.y + y + yo) % map->tile_size)) / map->tile_size
    };

    if (map->layout[grid_pos.y * map->size.x + new_grid_pos.x] == '.')
        self->pos.x += x;

    if (map->layout[new_grid_pos.y * map->size.x + grid_pos.x] == '.')
        self->pos.y += y;
}


void entity_move_towards_player(struct Entity* self, struct Player* p, struct Map* map)
{
    float theta = atan2f(p->rect.y - self->pos.y, p->rect.x - self->pos.x);
    theta += fmod(rand(), 3.f) - 1.5f;
    entity_move(self, map, self->speed * cosf(theta), self->speed * sinf(theta));
}


void entity_die(struct Entity* self, SDL_Renderer* rend, int weapon)
{
    self->enemy_dead = true;
    clock_gettime(CLOCK_MONOTONIC, &self->enemy_death_time);

    SDL_DestroyTexture(self->sprite);

    switch (weapon)
    {
    case WEAPON_GUN:
        self->sprite = IMG_LoadTexture(rend, "res/gfx/shrek_dead_gun.png");
        break;
    case WEAPON_KNIFE:
        self->sprite = IMG_LoadTexture(rend, "res/gfx/shrek_dead_knife.png");
        break;
    }
}

