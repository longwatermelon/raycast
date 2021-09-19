#include "player.h"
#include <math.h>


struct Player* player_init(SDL_Point pos, int angle)
{
    struct Player* p = malloc(sizeof(struct Player));
    p->rect = (SDL_FRect){ .x = pos.x, .y = pos.y, .w = 10, .h = 10 };
    p->angle = angle;

    p->angle = 0.f;
    p->angle_change = 0.f;

    return p;
}


void player_cleanup(struct Player* p)
{
    free(p);
}


void player_render(struct Player* p, SDL_Renderer* rend)
{
    SDL_SetRenderDrawColor(rend, 200, 200, 150, 255);
    SDL_RenderFillRectF(rend, &p->rect);

    SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);

    SDL_Point center = { .x = p->rect.x + p->rect.w / 2, .y = p->rect.y + p->rect.h / 2 };
    SDL_RenderDrawLine(rend, center.x, center.y, center.x + 10 * cosf(p->angle), center.y + 10 * sinf(p->angle));
}


void player_move(struct Player* p, char* map, int map_width, int tile_size)
{
    SDL_FPoint moved = {
        .x = p->speed * cosf(p->angle),
        .y = p->speed * sinf(p->angle)
    };

    int xo = (moved.x > 0 ? p->rect.w : 0);
    int yo = (moved.y > 0 ? p->rect.h : 0);

    SDL_Point grid_pos = {
        (int)((p->rect.x + xo) - ((int)(p->rect.x + xo) % tile_size)) / tile_size,
        (int)((p->rect.y + yo) - ((int)(p->rect.y + yo) % tile_size)) / tile_size
    };

    SDL_Point new_grid_pos = {
        (int)((p->rect.x + xo + moved.x) - ((int)(p->rect.x + xo + moved.x) % tile_size)) / tile_size,
        (int)((p->rect.y + yo + moved.y) - ((int)(p->rect.y + yo + moved.y) % tile_size)) / tile_size
    };

    // Separate x and y collision checks so that player can still move in directions that aren't occupied by obstacles after colliding with something
    if (map[grid_pos.y * map_width + new_grid_pos.x] != '#')
        p->rect.x += moved.x;

    if (map[new_grid_pos.y * map_width + grid_pos.x] != '#')
        p->rect.y += moved.y;


    p->angle += p->angle_change;

    // Keep angle between 0 and 2pi
    if (p->angle >= 2.f * M_PI)
        p->angle = 0.f;

    if (p->angle < 0.f)
        p->angle = 2.f * M_PI;
}

