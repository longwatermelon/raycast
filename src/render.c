#include "render.h"
#include "prog.h"
#include "common.h"


void render_3d_all(struct Prog* p)
{
    float x_pos = 0.f;

    for (float i = p->player->angle + M_PI / 6.f; i > p->player->angle - M_PI / 6.f; i -= 0.0013f) // Cast 800 rays
    {
        int ray_length_wall = render_3d_wall(p, i, x_pos);
        render_3d_entity(p, i, x_pos, ray_length_wall);

        x_pos += 1.f;
    }
}


int render_3d_wall(struct Prog* p, float angle, int col)
{
    int collision_type;
    SDL_Point endp = player_cast_ray(p->player, angle, p->map, p->entities, p->entities_size, &collision_type);

    int ray_length_wall = sqrtf((endp.x - p->player->rect.x) * (endp.x - p->player->rect.x) + (endp.y - p->player->rect.y) * (endp.y - p->player->rect.y));

    float angle_diff = common_restrict_angle(p->player->angle - angle);

    // Adjust for fisheye effect
    float dist = ray_length_wall * cosf(angle_diff);
    float line_height = (p->map->tile_size * 800.f) / dist;

    float line_offset = 400.f - line_height / 2.f;

    // Render walls
    SDL_Rect src = (SDL_Rect){
        .x = ((float)((collision_type == COLLISION_HORIZONTAL ? endp.x : endp.y) % p->map->tile_size) / (float)p->map->tile_size) * p->image_size.x,
        .y = 0,
        .w = 1,
        .h = p->image_size.y
    };

    SDL_Rect dst = (SDL_Rect){ .x = col, .y = (int)line_offset, .w = 1, .h = (int)line_height };
    SDL_RenderCopy(p->rend, p->tile_texture, &src, &dst);

    return ray_length_wall;
}


void render_3d_entity(struct Prog* p, float angle, int col, int ray_length_wall)
{
    float angle_diff = common_restrict_angle(p->player->angle - angle);

    struct Entity** ignored_entities = malloc(0);
    size_t ignored_entities_size = 0;

    int* entity_ray_lengths = malloc(0);
    size_t entity_ray_lengths_size = 0;

    float* intersections = malloc(0);
    size_t intersections_size = 0;

    for (int j = 0; j < p->entities_size; ++j)
    {
        float intersection;
        struct Entity* entity_hit;
        int ray_length_entity = player_cast_ray_entity(p->player, angle, p->entities, p->entities_size, ignored_entities, ignored_entities_size, &intersection, &entity_hit);

        ++ignored_entities_size;
        ignored_entities = realloc(ignored_entities, ignored_entities_size * sizeof(struct Entity*));
        ignored_entities[ignored_entities_size - 1] = entity_hit;

        ++entity_ray_lengths_size;
        entity_ray_lengths = realloc(entity_ray_lengths, sizeof(int) * entity_ray_lengths_size);
        entity_ray_lengths[entity_ray_lengths_size - 1] = ray_length_entity;

        ++intersections_size;
        intersections = realloc(intersections, sizeof(float) * intersections_size);
        intersections[intersections_size - 1] = intersection;
    }

    // Sort ray lengths
    for (int j = 0; j < entity_ray_lengths_size; ++j)
    {
        for (int k = j; k < entity_ray_lengths_size; ++k)
        {
            if (entity_ray_lengths[k] > entity_ray_lengths[j])
            {
                int tmpl = entity_ray_lengths[j];
                entity_ray_lengths[j] = entity_ray_lengths[k];
                entity_ray_lengths[k] = tmpl;

                float tmpi = intersections[j];
                intersections[j] = intersections[k];
                intersections[k] = tmpi;

                struct Entity* tmpe = ignored_entities[j];
                ignored_entities[j] = ignored_entities[k];
                ignored_entities[k] = tmpe;
            }
        }
    }

    SDL_Rect src = { .y = 0, .w = 1 };
    SDL_Rect dst = { .x = col, .w = 1 };

    // Render entities
    for (int j = 0; j < entity_ray_lengths_size; ++j)
    {
        if (entity_ray_lengths[j] < ray_length_wall && entity_ray_lengths[j] != -1)
        {
            src.x = (intersections[j] / ignored_entities[j]->width) * ignored_entities[j]->sprite_size.x;
            src.h = ignored_entities[j]->sprite_size.y;

            float dist = entity_ray_lengths[j] * cosf(angle_diff);
            float line_height = (25.f * 800.f) / dist;

            float line_offset = 400.f;

            dst.y = line_offset;
            dst.h = line_height;

            SDL_RenderCopy(p->rend, ignored_entities[j]->sprite, &src, &dst);
        }
    }

    free(ignored_entities);
    free(entity_ray_lengths);
    free(intersections);
}

