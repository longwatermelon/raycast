#include "entity.h"


struct Entity* entity_init(SDL_Point pos)
{
    struct Entity* e = malloc(sizeof(struct Entity));
    e->pos = pos;

    return e;
}


void entity_cleanup(struct Entity* e)
{
    free(e);
}

