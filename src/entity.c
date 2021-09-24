#include "entity.h"
#include <SDL_image.h>


struct Entity* entity_init(SDL_Point pos, SDL_Renderer* rend, const char* sprite_path)
{
    struct Entity* e = malloc(sizeof(struct Entity));
    e->pos = pos;
    e->sprite = IMG_LoadTexture(rend, sprite_path);

    return e;
}


void entity_cleanup(struct Entity* e)
{
    SDL_DestroyTexture(e->sprite);
    free(e);
}

