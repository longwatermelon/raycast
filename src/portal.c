#include "portal.h"
#include "common.h"


struct Portal *portal_alloc(SDL_Point grid_pos, int dir)
{
    struct Portal *p = malloc(sizeof(struct Portal));
    p->grid_pos = grid_pos;
    p->dir = dir;

    if (dir == DIR_RIGHT)
        p->grid_pos.x -= 1;

    if (dir == DIR_DOWN)
        p->grid_pos.y -= 1;

    return p;
}


void portal_free(struct Portal *p)
{
    free(p);
}


bool portal_check_collision(struct Portal *p, SDL_Point grid_pos, int dir)
{
    switch (p->dir)
    {
    case DIR_UP: break;
    case DIR_DOWN: grid_pos.y -= 1; break;
    case DIR_LEFT: break;
    case DIR_RIGHT: grid_pos.x -= 1; break;
    }

    return p->dir == dir && p->grid_pos.x == grid_pos.x && p->grid_pos.y == grid_pos.y;
}

