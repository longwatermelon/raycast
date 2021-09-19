#include "player.h"
#include <math.h>
#include <stdbool.h>


struct Player* player_init(SDL_Point pos, float angle)
{
    struct Player* p = malloc(sizeof(struct Player));
    p->rect = (SDL_FRect){ .x = pos.x, .y = pos.y, .w = 10, .h = 10 };
    p->angle = angle;

    p->angle = angle;
    p->angle_change = 0.f;

    return p;
}


void player_cleanup(struct Player* p)
{
    free(p);
}


void player_render(struct Player* p, SDL_Renderer* rend, char* map, int map_width, int tile_size)
{
    SDL_SetRenderDrawColor(rend, 200, 200, 150, 255);
    SDL_RenderFillRectF(rend, &p->rect);

    SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);

    SDL_Point center = { .x = p->rect.x + p->rect.w / 2, .y = p->rect.y + p->rect.h / 2 };
    SDL_RenderDrawLine(rend, center.x, center.y, center.x + 10 * cosf(p->angle), center.y + 10 * -sinf(p->angle));

    /* printf("%f\n", sinf(M_PI / 2.f)); */
    /* printf("%f %f\n", p->angle, 10 * sinf(p->angle)); */
    /* printf("%f\n", 10 * sinf(p->angle)); */
    SDL_Point endp = player_cast_ray(p, p->angle, map, map_width, tile_size);
    SDL_RenderDrawLine(rend, center.x, center.y, endp.x, endp.y);
}


void player_move(struct Player* p, char* map, int map_width, int tile_size)
{
    SDL_FPoint moved = {
        .x = p->speed * cosf(p->angle),
        .y = p->speed * -sinf(p->angle)
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
    if (p->angle > 2.f * M_PI)
        p->angle -= 2.f * M_PI;

    if (p->angle < 0.f)
        p->angle += 2.f * M_PI;
}


SDL_Point player_cast_ray(struct Player* p, float angle, char* map, int map_width, int tile_size)
{
    SDL_Point horizontal = player_cast_ray_horizontal(p, angle, map, map_width, tile_size);
    SDL_Point vertical = player_cast_ray_vertical(p, angle, map, map_width, tile_size);

    // TODO find shortest ray

    return horizontal;
}


SDL_Point player_cast_ray_horizontal(struct Player* p, float angle, char* map, int map_width, int tile_size)
{
    // Cast ray that only intersects horizontal lines

    SDL_Point closest_horizontal;
    closest_horizontal.y = (int)p->rect.y - ((int)p->rect.y % tile_size);
    closest_horizontal.x = p->rect.x + ((closest_horizontal.y - p->rect.y) / -tanf(angle));

    if (angle <= 0.01f || angle >= 2 * M_PI - 0.01f || fabsf((float)M_PI - angle) <= 0.01f) // Facing right, almost undefined
        return closest_horizontal;

    while (true)
    {
        SDL_Point grid_pos = {
            .x = (closest_horizontal.x - (closest_horizontal.x % tile_size)) / tile_size,
            .y = (closest_horizontal.y - (closest_horizontal.y % tile_size)) / tile_size
        };

        if (grid_pos.y < 0 || grid_pos.y >= strlen(map) / map_width || grid_pos.x < 0 || grid_pos.x >= map_width)
            return closest_horizontal;

        if (map[grid_pos.y * map_width + grid_pos.x] == '#')
        {
            return closest_horizontal;
        }

        int dy = (angle < M_PI ? -tile_size : tile_size);

        closest_horizontal.y += dy;
        closest_horizontal.x += dy / -tanf(angle);
    }
}


SDL_Point player_cast_ray_vertical(struct Player* p, float angle, char* map, int map_width, int tile_size)
{
}

