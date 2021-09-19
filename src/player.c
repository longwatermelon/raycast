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

    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);

    SDL_Point center = { .x = p->rect.x + p->rect.w / 2, .y = p->rect.y + p->rect.h / 2 };
    SDL_RenderDrawLine(rend, center.x, center.y, center.x + 10 * cosf(p->angle), center.y + 10 * -sinf(p->angle));

    SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
    /* printf("%f\n", sinf(M_PI / 2.f)); */
    /* printf("%f %f\n", p->angle, 10 * sinf(p->angle)); */
    /* printf("%f\n", 10 * sinf(p->angle)); */

    for (float i = p->angle - M_PI / 5.f; i < p->angle + M_PI / 5.f; i += 0.01f)
    {
        SDL_Point endp = player_cast_ray(p, i, map, map_width, tile_size);
        SDL_RenderDrawLine(rend, center.x, center.y, endp.x, endp.y);
    }
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

    SDL_Point diff_h = { .x = horizontal.x - p->rect.x, .y = horizontal.y - p->rect.y };
    SDL_Point diff_v = { .x = vertical.x - p->rect.x, .y = vertical.y - p->rect.y };

    int dist_h = sqrtf(diff_h.x * diff_h.x + diff_h.y * diff_h.y);
    int dist_v = sqrtf(diff_v.x * diff_v.x + diff_v.y * diff_v.y);

    return dist_h < dist_v ? horizontal : vertical;
    /* return horizontal; */
}


SDL_Point player_cast_ray_horizontal(struct Player* p, float angle, char* map, int map_width, int tile_size)
{
    // Cast ray that only intersects horizontal lines

    SDL_Point closest_horizontal;
    closest_horizontal.y = (int)p->rect.y - ((int)p->rect.y % tile_size) + (angle > M_PI ? tile_size : 0);
    closest_horizontal.x = p->rect.x + ((closest_horizontal.y - p->rect.y) / -tanf(angle));

    if (angle <= 0.001f || 2 * M_PI - angle <= 0.001f) // Facing right, almost undefined
    {
        return (SDL_Point){ 800, p->rect.y };
    }

    if (fabsf((float)M_PI - angle) <= 0.001f) // Facing left, almost undefined
    {
        return (SDL_Point){ -800, p->rect.y };
    }

    while (true)
    {
        SDL_Point grid_pos = {
            .x = (closest_horizontal.x - (closest_horizontal.x % tile_size)) / tile_size,
            .y = (closest_horizontal.y - (closest_horizontal.y % tile_size)) / tile_size
        };

        // Out of bounds, no point in continuing
        if (grid_pos.y < 0 || grid_pos.y >= strlen(map) / map_width || grid_pos.x < 0 || grid_pos.x >= map_width)
        {
            return closest_horizontal;
        }

        if (map[(grid_pos.y - 1) * map_width + grid_pos.x] == '#' || map[grid_pos.y * map_width + grid_pos.x] == '#')
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
    // Cast ray that only intersects vertical lines
    
    SDL_Point closest_vertical;
    closest_vertical.x = (int)p->rect.x - ((int)p->rect.x % tile_size) + (angle < M_PI / 2.f || angle > 3 * M_PI / 2.f ? tile_size : 0);
    closest_vertical.y = p->rect.y + ((closest_vertical.x - p->rect.x) * -tanf(angle));

    if (fabsf((float)(M_PI / 2.f) - angle) <= 0.001f)
        return (SDL_Point){ p->rect.x, -800 };

    if (fabsf((float)(3 * M_PI / 2.f) - angle) <= 0.001f)
        return (SDL_Point){ p->rect.x, 800 };

    while (true)
    {
        SDL_Point grid_pos = {
            .x = (closest_vertical.x - (closest_vertical.x % tile_size)) / tile_size,
            .y = (closest_vertical.y - (closest_vertical.y % tile_size)) / tile_size
        };

         // Out of bounds, no point in continuing
        if (grid_pos.y < 0 || grid_pos.y >= strlen(map) / map_width || grid_pos.x < 0 || grid_pos.x >= map_width)
        {
            return closest_vertical;
        }

        if (map[grid_pos.y * map_width + (grid_pos.x - 1)] == '#' || map[grid_pos.y * map_width + grid_pos.x] == '#')
        {
            return closest_vertical;
        }

        int dx = (angle < M_PI / 2.f || angle > 3 * M_PI / 2.f ? tile_size : -tile_size);

        closest_vertical.x += dx;
        closest_vertical.y += dx * -tanf(angle);
    }
}

