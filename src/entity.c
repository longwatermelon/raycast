#include "entity.h"
#include "player.h"
#include "common.h"
#include <SDL_image.h>


struct Entity* entity_init(SDL_FPoint pos, SDL_Renderer* rend, const char* sprite_path)
{
    struct Entity* e = malloc(sizeof(struct Entity));
    e->pos = pos;

    e->sprite = IMG_LoadTexture(rend, sprite_path);
    SDL_QueryTexture(e->sprite, 0, 0, &e->sprite_size.x, &e->sprite_size.y);

    return e;
}


void entity_cleanup(struct Entity* e)
{
    SDL_DestroyTexture(e->sprite);
    free(e);
}


void entity_move(struct Entity* e, float x, float y)
{
    e->pos.x += x;
    e->pos.y += y;
}


void entity_move_towards_player(struct Entity* e, struct Player* p, struct Map* map)
{
    float theta = atan2f(p->rect.y - e->pos.y, p->rect.x - e->pos.x);
    entity_move(e, cosf(theta), sinf(theta));
}

