#include "render.h"
#include "prog.h"
#include "common.h"
#include <SDL2/SDL_blendmode.h>


void render_3d_all(struct Prog  *p)
{
    float x_pos = 0.f;

    for (float i = p->player->angle + M_PI / 6.f; i > p->player->angle - M_PI / 6.f; i -= 0.0013f) // Cast 800 rays
    {
        int ray_length_wall = render_3d_wall(p, i, x_pos);
        render_3d_entity(p, i, x_pos, ray_length_wall);

        x_pos += 1.f;
    }
}


int render_3d_wall(struct Prog *p, float angle, int col)
{
    int collision_type;
    SDL_Point endp = player_cast_ray(p->player, angle, p->map, p->entities, p->entities_size, &collision_type);

    int ray_length_wall = sqrtf((endp.x - p->player->pos.x) * (endp.x - p->player->pos.x) + (endp.y - p->player->pos.y) * (endp.y - p->player->pos.y));

    float angle_diff = common_restrict_angle(p->player->angle - angle);

    // Adjust for fisheye effect
    float dist = ray_length_wall;

    if (p->adjust_fisheye)
        dist = ray_length_wall * cosf(angle_diff);

    float line_height = (p->map->tile_size * 800.f) / dist;

    float line_offset = 400.f - line_height / 2.f;

    bool horizontal = collision_type == DIR_LEFT || collision_type == DIR_RIGHT;

    // Render walls
    SDL_Rect src = (SDL_Rect){
        .x = ((float)((horizontal ? endp.x : endp.y) % p->map->tile_size) / (float)p->map->tile_size) * p->image_size.x,
        .y = 0,
        .w = 1,
        .h = p->image_size.y
    };

    SDL_Rect dst = (SDL_Rect){ .x = col, .y = (int)line_offset, .w = 1, .h = (int)line_height };
    SDL_RenderCopy(p->rend, p->tile_texture, &src, &dst);

    int opacity = (horizontal ? 35 : 0);
    SDL_SetRenderDrawBlendMode(p->rend, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(p->rend, 0, 0, 0, opacity);
    SDL_RenderFillRect(p->rend, &dst);
    SDL_SetRenderDrawBlendMode(p->rend, SDL_BLENDMODE_NONE);

    return ray_length_wall;
}


void render_3d_entity(struct Prog *p, float angle, int col, int ray_length_wall)
{
    float angle_diff = common_restrict_angle(p->player->angle - angle);

    // + 1 because MAX_ENTITIES does not account for nuts
    struct Entity *rendered_entities[MAX_ENTITIES + 1];
    int entity_ray_lengths[MAX_ENTITIES + 1];
    float intersections[MAX_ENTITIES + 1];

    size_t intersection_num = 0;

    for (int j = 0; j < p->entities_size; ++j)
    {
        float intersection = 0.f;
        struct Entity *entity_hit = 0;
        int ray_length_entity = player_cast_ray_entity(p->player, angle, p->entities, p->entities_size, rendered_entities, intersection_num, -1, &intersection, &entity_hit);

        rendered_entities[intersection_num] = entity_hit;
        entity_ray_lengths[intersection_num] = ray_length_entity;
        intersections[intersection_num] = intersection;
        ++intersection_num;
    }

    // Sort ray lengths
    for (int j = 0; j < intersection_num; ++j)
    {
        for (int k = j; k < intersection_num; ++k)
        {
            if (entity_ray_lengths[k] > entity_ray_lengths[j])
            {
                int tmpl = entity_ray_lengths[j];
                entity_ray_lengths[j] = entity_ray_lengths[k];
                entity_ray_lengths[k] = tmpl;

                float tmpi = intersections[j];
                intersections[j] = intersections[k];
                intersections[k] = tmpi;

                struct Entity *tmpe = rendered_entities[j];
                rendered_entities[j] = rendered_entities[k];
                rendered_entities[k] = tmpe;
            }
        }
    }

    SDL_Rect src = { .y = 0, .w = 1 };
    SDL_Rect dst = { .x = col, .w = 1 };

    float cos_angle_diff = cosf(angle_diff);

    // Render entities
    for (int j = 0; j < intersection_num; ++j)
    {
        if ((p->render_entities_over_walls && rendered_entities[j]) || (entity_ray_lengths[j] < ray_length_wall && entity_ray_lengths[j] != -1))
        {
            src.x = (intersections[j] / rendered_entities[j]->width) * rendered_entities[j]->sprite_size.x;
            src.h = rendered_entities[j]->sprite_size.y;

            float dist = entity_ray_lengths[j];

            if (p->adjust_fisheye)
                dist = entity_ray_lengths[j] * cos_angle_diff;

            float line_height = (25.f * 800.f) / dist;

            float line_offset = 400.f;

            dst.y = line_offset;
            dst.h = line_height;

            SDL_RenderCopy(p->rend, rendered_entities[j]->sprite, &src, &dst);

            if (p->render_entities_over_walls && (!(entity_ray_lengths[j] < ray_length_wall) || !(entity_ray_lengths[j] != -1)))
            {
                SDL_SetRenderDrawBlendMode(p->rend, SDL_BLENDMODE_BLEND);

                SDL_Color color = { .r = 0, .g = 0, .b = 0 };

                switch (rendered_entities[j]->type)
                {
                case ENTITY_NUTS:
                    color = (SDL_Color){ .r = 0, .g = 255, .b = 0 };
                    break;
                case ENTITY_ENEMY:
                    color = (SDL_Color){ .r = 0, .g = 0, .b = 0 };
                    break;
                case ENTITY_AMMO:
                    color = (SDL_Color){ .r = 255, .g = 0, .b = 255 };
                    break;
                }

                SDL_SetRenderDrawColor(p->rend, color.r, color.g, color.b, 160);
                SDL_RenderFillRect(p->rend, &dst);
                SDL_SetRenderDrawBlendMode(p->rend, SDL_BLENDMODE_NONE);
            }
        }
    }
}

