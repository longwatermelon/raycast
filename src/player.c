#include "player.h"
#include "common.h"
#include <math.h>
#include <time.h>
#include <stdbool.h>


struct Player* player_init(SDL_Point pos, float angle)
{
    // Valgrind shits itself if I use malloc instead of calloc, no idea why
    struct Player* p = calloc(1, sizeof(struct Player));
    p->rect = (SDL_FRect){ .x = pos.x, .y = pos.y, .w = 10, .h = 10 };
    p->angle = angle;

    p->angle = angle;
    p->angle_change = 0.f;

    p->ray_mode = RAY_ALL;

    p->alive = true;

    p->shooting = false;
    p->last_shot_time = clock();
    p->reloading = false;
    p->bullets = 100;
    p->bullets_loaded = 20;

    return p;
}


void player_cleanup(struct Player* p)
{
    free(p);
}


void player_render(struct Player* p, SDL_Renderer* rend, struct Map* map, struct Entity** entities, size_t entities_size)
{
    SDL_SetRenderDrawColor(rend, 200, 200, 150, 255);
    SDL_RenderFillRectF(rend, &p->rect);

    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);

    SDL_Point center = { .x = p->rect.x + p->rect.w / 2, .y = p->rect.y + p->rect.h / 2 };
    SDL_RenderDrawLine(rend, center.x, center.y, center.x + 10 * cosf(p->angle), center.y + 10 * -sinf(p->angle));

    SDL_SetRenderDrawColor(rend, 0, 0, 255, 255);

    for (float i = p->angle - M_PI / 6.f; i < p->angle + M_PI / 6.f; i += 0.0013f)
    {
        int collision_type;
        SDL_Point endp = player_cast_ray(p, i, map, entities, entities_size, &collision_type);

        float intersection;
        struct Entity* entity_hit = 0;
        float enitity_length = player_cast_ray_entity(p, i, entities, entities_size, 0, 0, -1, &intersection, &entity_hit);

        if (collision_type == COLLISION_HORIZONTAL)
            SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
        else if (collision_type == COLLISION_VERTICAL)
            SDL_SetRenderDrawColor(rend, 0, 255, 0, 255);

        SDL_Point diff = {
            endp.x - p->rect.x,
            endp.y - p->rect.y
        };

        float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);

        if (enitity_length < dist && enitity_length != -1)
        {
            SDL_SetRenderDrawColor(rend, 255, 0, 255, 255);
            SDL_RenderDrawLine(rend, center.x, center.y, endp.x, endp.y);
        }
    }

    for (int i = 0; i < entities_size; ++i)
    {
        SDL_Rect rect = { .x = entities[i]->pos.x - 5, .y = entities[i]->pos.y - 5, .w = 10, .h = 10 };
        SDL_RenderFillRect(rend, &rect);
    }
}


void player_move(struct Player* p, struct Map* map)
{
    SDL_FPoint moved = {
        .x = p->speed * cosf(p->angle),
        .y = p->speed * -sinf(p->angle)
    };

    if (p->reloading)
    {
        moved.x *= 0.5f;
        moved.y *= 0.5f;
    }

    int xo = (moved.x > 0 ? p->rect.w + 5 : -5);
    int yo = (moved.y > 0 ? p->rect.h + 5 : -5);

    SDL_Point grid_pos = {
        (int)((p->rect.x + xo) - ((int)(p->rect.x + xo) % map->tile_size)) / map->tile_size,
        (int)((p->rect.y + yo) - ((int)(p->rect.y + yo) % map->tile_size)) / map->tile_size
    };

    SDL_Point new_grid_pos = {
        (int)((p->rect.x + xo + moved.x) - ((int)(p->rect.x + xo + moved.x) % map->tile_size)) / map->tile_size,
        (int)((p->rect.y + yo + moved.y) - ((int)(p->rect.y + yo + moved.y) % map->tile_size)) / map->tile_size
    };

    // Separate x and y collision checks so that player can still move in directions that aren't occupied by obstacles after colliding with something
    if (map->layout[grid_pos.y * map->size.x + new_grid_pos.x] != '#')
        p->rect.x += moved.x;

    if (map->layout[new_grid_pos.y * map->size.x + grid_pos.x] != '#')
        p->rect.y += moved.y;

    p->angle += p->angle_change;

    // Keep angle between 0 and 2pi
    p->angle = common_restrict_angle(p->angle);
}


SDL_Point player_cast_ray(struct Player* p, float angle, struct Map* map, struct Entity** entities, size_t entities_size, int* collision_type)
{
    if (angle > 2.f * M_PI)
        angle -= 2.f * M_PI;

    if (angle < 0.f)
        angle += 2.f * M_PI;

    SDL_Point horizontal = player_cast_ray_horizontal(p, angle, map);
    SDL_Point vertical = player_cast_ray_vertical(p, angle, map);

    SDL_Point diff_h = { .x = horizontal.x - p->rect.x, .y = horizontal.y - p->rect.y };
    SDL_Point diff_v = { .x = vertical.x - p->rect.x, .y = vertical.y - p->rect.y };

    unsigned long dist_h = sqrtf(diff_h.x * diff_h.x + diff_h.y * diff_h.y);
    unsigned long dist_v = sqrtf(diff_v.x * diff_v.x + diff_v.y * diff_v.y);

    if (dist_h < dist_v)
        *collision_type = COLLISION_HORIZONTAL;
    else
        *collision_type = COLLISION_VERTICAL;

    if (p->ray_mode == RAY_HORIZONTAL)
        return horizontal;
    if (p->ray_mode == RAY_VERTICAL)
        return vertical;

    return dist_h < dist_v ? horizontal : vertical;
}


SDL_Point player_cast_ray_horizontal(struct Player* p, float angle, struct Map* map)
{
    // Cast ray that only intersects horizontal lines

    SDL_FPoint closest_horizontal;
    closest_horizontal.y = (int)p->rect.y - ((int)p->rect.y % map->tile_size) + (angle > M_PI ? map->tile_size : 0);
    closest_horizontal.x = p->rect.x + ((closest_horizontal.y - p->rect.y) / -tanf(angle));

    if (angle <= 0.001f || 2 * M_PI - angle <= 0.001f) // Facing right, almost undefined
        return (SDL_Point){ 1e5, p->rect.y };

    if (fabsf((float)M_PI - angle) <= 0.001f) // Facing left, almost undefined
        return (SDL_Point){ -1e5, p->rect.y };

    while (true)
    {
        SDL_Point grid_pos = {
            .x = (closest_horizontal.x - (fmod(closest_horizontal.x, map->tile_size))) / map->tile_size,
            .y = (closest_horizontal.y - (fmod(closest_horizontal.y, map->tile_size))) / map->tile_size
        };

        if (angle < M_PI)
            grid_pos.y -= 1;

        // Out of bounds, no point in continuing
        if (grid_pos.y < 0 || grid_pos.y >= map->size.y || grid_pos.x < 0 || grid_pos.x >= map->size.x)
            return (SDL_Point){ .x = (int)closest_horizontal.x, .y = (int)closest_horizontal.y };

        if (map->layout[grid_pos.y * map->size.x + grid_pos.x] == '#')
            return (SDL_Point){ .x = (int)closest_horizontal.x, .y = (int)closest_horizontal.y };

        float dy = (angle < M_PI ? -map->tile_size : map->tile_size);

        closest_horizontal.y += dy;
        closest_horizontal.x += dy / -tanf(angle);
    }
}


SDL_Point player_cast_ray_vertical(struct Player* p, float angle, struct Map* map)
{
    // Cast ray that only intersects vertical lines
    
    SDL_FPoint closest_vertical;
    closest_vertical.x = (int)p->rect.x - ((int)p->rect.x % map->tile_size) + (angle < M_PI / 2.f || angle > 3 * M_PI / 2.f ? map->tile_size : 0);
    closest_vertical.y = p->rect.y + ((closest_vertical.x - p->rect.x) * -tanf(angle));

    if (fabsf((float)(M_PI / 2.f) - angle) <= 0.001f)
        return (SDL_Point){ p->rect.x, -1e5 };

    if (fabsf((float)(3 * M_PI / 2.f) - angle) <= 0.001f)
        return (SDL_Point){ p->rect.x, 1e5 };

    while (true)
    {
        SDL_Point grid_pos = {
            .x = (closest_vertical.x - (fmod(closest_vertical.x, map->tile_size))) / map->tile_size,
            .y = (closest_vertical.y - (fmod(closest_vertical.y, map->tile_size))) / map->tile_size
        };

        if (angle > M_PI / 2.f && angle < 3 * M_PI / 2.f)
            grid_pos.x -= 1;

         // Out of bounds, no point in continuing
        if (grid_pos.y < 0 || grid_pos.y >= map->size.y || grid_pos.x < 0 || grid_pos.x >= map->size.x)
            return (SDL_Point){ .x = (int)closest_vertical.x, .y = (int)closest_vertical.y };

        if (map->layout[grid_pos.y * map->size.x + grid_pos.x] == '#')
            return (SDL_Point){ .x = (int)closest_vertical.x, .y = (int)closest_vertical.y };

        float dx = (angle < M_PI / 2.f || angle > 3 * M_PI / 2.f ? map->tile_size : -map->tile_size);

        closest_vertical.x += dx;
        closest_vertical.y += dx * -tanf(angle);
    }
}


int player_cast_ray_entity(struct Player* p, float angle, struct Entity** entities, size_t entities_size, struct Entity** ignored_entities, size_t ignored_entities_size, int target_type, float* intersection, struct Entity** entity_hit)
{
    float shortest = -1;

    for (int i = 0; i < entities_size; ++i)
    {
        if (target_type != -1 && entities[i]->type != target_type)
            continue;

        bool is_ignored = false;

        for (int j = 0; j < ignored_entities_size; ++j)
        {
            if (entities[i] == ignored_entities[j])
            {
                is_ignored = true;
                break;
            }
        }

        if (is_ignored)
            continue;

        SDL_FPoint diff = {
            .x = entities[i]->pos.x - p->rect.x,
            .y = entities[i]->pos.y - p->rect.y
        };

        SDL_FPoint ray_vector = {
            .x = cosf(angle),
            .y = -sinf(angle)
        };

        float dot_product = diff.x * ray_vector.x + diff.y * ray_vector.y;
        float dist_a = sqrtf(diff.x * diff.x + diff.y * diff.y);

        float cos_theta = dot_product / dist_a;
        if (cos_theta > 1.f)
            cos_theta = 1.f;

        if (cos_theta < -1.f)
            cos_theta = -1.f;

        float theta = acosf(cos_theta);

        float cross = diff.x * ray_vector.y - diff.y * ray_vector.x;

        if (theta < M_PI / 2.f)
        {
            float h = dist_a * tanf(theta);

            if (h <= entities[i]->width / 2.f)
            {
                float len = sqrtf(dist_a * dist_a + h * h);

                if (len < shortest || shortest == -1)
                {
                    shortest = len;

                    if (cross < 0)
                        *intersection = (entities[i]->width / 2.f) - h;
                    else
                        *intersection = (entities[i]->width / 2.f) + h;

                    *entity_hit = entities[i];
                }
            }
        }
    }

    return shortest;
}

