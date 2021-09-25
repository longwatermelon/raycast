#include "entity.h"
#include "player.h"
#include "common.h"
#include <time.h>
#include <SDL_image.h>


struct Entity* entity_init(SDL_FPoint pos, SDL_Renderer* rend, const char* sprite_path)
{
    struct Entity* e = malloc(sizeof(struct Entity));
    e->pos = pos;

    e->sprite = IMG_LoadTexture(rend, sprite_path);
    SDL_QueryTexture(e->sprite, 0, 0, &e->sprite_size.x, &e->sprite_size.y);

    e->last_hit = clock();

    return e;
}


void entity_cleanup(struct Entity* e)
{
    SDL_DestroyTexture(e->sprite);
    free(e);
}


void entity_move(struct Entity* e, struct Map* map, float x, float y)
{
    clock_t t2 = clock();

    SDL_Point grid_pos = {
        (int)(e->pos.x - ((int)(e->pos.x) % map->tile_size)) / map->tile_size,
        (int)(e->pos.y - ((int)(e->pos.y) % map->tile_size)) / map->tile_size
    };

    SDL_Point new_grid_pos = {
        (int)((e->pos.x + x) - ((int)(e->pos.x + x) % map->tile_size)) / map->tile_size,
        (int)((e->pos.y + y) - ((int)(e->pos.y + y) % map->tile_size)) / map->tile_size
    };

    if (map->layout[grid_pos.y * map->size.x + new_grid_pos.x] != '#')
        e->pos.x += x;

    if (map->layout[new_grid_pos.y * map->size.x + grid_pos.x] != '#')
        e->pos.y += y;
}


void entity_move_towards_player(struct Entity* e, struct Player* p, struct Map* map)
{
    float theta = atan2f(p->rect.y - e->pos.y, p->rect.x - e->pos.x);
    entity_move(e, map, cosf(theta), sinf(theta));
}

