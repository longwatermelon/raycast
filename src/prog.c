#include "prog.h"
#include "common.h"
#include "entity.h"
#include "player.h"
#include <time.h>
#include <SDL_image.h>


struct Prog* prog_init()
{
    struct Prog* p = malloc(sizeof(struct Prog));
    p->running = true;

    p->window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, SDL_WINDOW_SHOWN);
    p->rend = SDL_CreateRenderer(p->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    p->font = TTF_OpenFont("res/font.ttf", 16);

    p->player = player_init((SDL_Point){ 300, 320 }, M_PI);
    p->map = map_init("map", (SDL_Point){ 32, 32 }, 50);

    p->entities = malloc(0);
    p->entities_size = 0;

    p->tile_texture = IMG_LoadTexture(p->rend, "res/wall.png");
    SDL_QueryTexture(p->tile_texture, 0, 0, &p->image_size.x, &p->image_size.y);

    p->gun_texture = IMG_LoadTexture(p->rend, "res/gun.png");
    p->shot_texture = IMG_LoadTexture(p->rend, "res/gun_shoot.png");

    return p;
}


void prog_cleanup(struct Prog* p)
{
    SDL_DestroyTexture(p->tile_texture);
    SDL_DestroyTexture(p->gun_texture);

    TTF_CloseFont(p->font);

    player_cleanup(p->player);
    map_cleanup(p->map);

    for (int i = 0; i < p->entities_size; ++i)
        entity_cleanup(p->entities[i]);

    free(p->entities);

    SDL_DestroyRenderer(p->rend);
    SDL_DestroyWindow(p->window);

    free(p);
}


void prog_mainloop(struct Prog* p)
{
    SDL_Event evt;

    while (p->running)
    {
        prog_handle_events(p, &evt);

        player_move(p->player, p->map);

        for (int i = 0; i < p->entities_size; ++i)
        {
            entity_move_towards_player(p->entities[i], p->player, p->map);
        }

        if (p->player->shooting)
        {
            if ((float)(clock() - p->player->last_shot_time) / CLOCKS_PER_SEC >= .01f)
            {
                p->player->shooting = false;
            }
        }

        if (p->entities_size < 15 && rand() % 2000 > 1985)
            prog_add_entity(p);

        SDL_RenderClear(p->rend);

        prog_render_3d(p);
        prog_render_gun(p);

        // TODO move this to a function
        {
            int bullet_num_len = snprintf(0, 0, "%d", p->player->bullets);
            int length = strlen("Bullets: ") + bullet_num_len + 1;
            char* bullets = malloc(sizeof(char) * length);
            snprintf(bullets, length, "Bullets: %d", p->player->bullets);
            SDL_Texture* ammo_counter_tex = common_render_text(p->rend, p->font, bullets);
            free(bullets);

            SDL_Rect tmp = { .x = 20, .y = 20 };
            SDL_QueryTexture(ammo_counter_tex, 0, 0, &tmp.w, &tmp.h);
            SDL_RenderCopy(p->rend, ammo_counter_tex, 0, &tmp);

            SDL_DestroyTexture(ammo_counter_tex);
        }

        {
            int enemies_num_len = snprintf(0, 0, "%ld", p->entities_size);
            int length = strlen("Enemies alive: ") + enemies_num_len + 1;
            char* enemies = malloc(sizeof(char) * length);
            snprintf(enemies, length, "Enemies alive: %ld", p->entities_size);
            SDL_Texture* enemy_counter_tex = common_render_text(p->rend, p->font, enemies);
            free(enemies);

            SDL_Rect tmp = { .x = 20, .y = 40 };
            SDL_QueryTexture(enemy_counter_tex, 0, 0, &tmp.w, &tmp.h);
            SDL_RenderCopy(p->rend, enemy_counter_tex, 0, &tmp);

            SDL_DestroyTexture(enemy_counter_tex);
        }
        /* if (p->player->bullets <= 0) */
        /* { */
        /*     SDL_Rect tmp = { .x = 20, .y = 20 }; */
        /*     SDL_QueryTexture(ammo_warning, 0, 0, &tmp.w, &tmp.h); */
        /*     SDL_RenderCopy(p->rend, ammo_warning, 0, &tmp); */
        /* } */

        /* prog_render_map(p); */
        /* player_render(p->player, p->rend, p->map, p->entities, p->entities_size); */

        SDL_Rect crosshair = { .x = 400 - 2, .y = 400 - 2, .w = 4, .h = 4 };
        SDL_SetRenderDrawColor(p->rend, 255, 0, 0, 255);
        SDL_RenderFillRect(p->rend, &crosshair);

        SDL_SetRenderDrawColor(p->rend, 0, 0, 0, 255);
        SDL_RenderPresent(p->rend);
    }
}


void prog_handle_events(struct Prog* p, SDL_Event* evt)
{
    float player_speed = 2.f;

    while (SDL_PollEvent(evt))
    {
        switch (evt->type)
        {
        case SDL_QUIT:
            p->running = false;
            break;
        case SDL_KEYDOWN:
        {
            switch (evt->key.keysym.sym)
            {
            case SDLK_UP:
                p->player->speed = player_speed;
                break;
            case SDLK_DOWN:
                p->player->speed = -player_speed;
                break;
            case SDLK_RIGHT:
                p->player->angle_change = -.02f;
                break;
            case SDLK_LEFT:
                p->player->angle_change = .02f;
                break;
            case SDLK_r:
                p->player->ray_mode = RAY_ALL;
                break;
            case SDLK_h:
                p->player->ray_mode = RAY_HORIZONTAL;
                break;
            case SDLK_v:
                p->player->ray_mode = RAY_VERTICAL;
                break;
            case SDLK_z:
            {
                if (p->player->bullets <= 0)
                    break;

                p->player->shooting = true;
                p->player->last_shot_time = clock();
                --p->player->bullets;

                float intersection;
                struct Entity* entity;
                int entity_dist = player_cast_ray_entity(p->player, p->player->angle, p->entities, p->entities_size, 0, 0, &intersection, &entity);

                int collision_type;
                SDL_Point wall_vector = player_cast_ray(p->player, p->player->angle, p->map, p->entities, p->entities_size, &collision_type);
                SDL_Point diff = { .x = wall_vector.x - p->player->rect.x, .y = wall_vector.y - p->player->rect.y };
                int wall_dist = sqrtf(diff.x * diff.x + diff.y * diff.y);

                if (entity_dist != -1 && entity_dist < wall_dist)
                {
                    prog_remove_entity(p, entity);
                }
            } break;
            case SDLK_x:
            {
                p->player->reloading = true;
            } break;
            }
        } break;
        case SDL_KEYUP:
        {
            switch (evt->key.keysym.sym)
            {
            case SDLK_UP:
            case SDLK_DOWN:
                p->player->speed = 0;
                break;
            case SDLK_RIGHT:
            case SDLK_LEFT:
                p->player->angle_change = 0.f;
                break;
            }
        } break;
        }
    }
}


void prog_render_3d(struct Prog* p)
{
    float x_pos = 0.f;

    for (float i = p->player->angle + M_PI / 6.f; i > p->player->angle - M_PI / 6.f; i -= 0.0013f) // Cast 800 rays
    {
        int collision_type;
        SDL_Point endp = player_cast_ray(p->player, i, p->map, p->entities, p->entities_size, &collision_type);

        int ray_length_wall = sqrtf((endp.x - p->player->rect.x) * (endp.x - p->player->rect.x) + (endp.y - p->player->rect.y) * (endp.y - p->player->rect.y));

        float angle = common_restrict_angle(p->player->angle - i);

        // Adjust for fisheye effect
        float dist = ray_length_wall * cosf(angle);
        float line_height = (p->map->tile_size * 800.f) / dist;

        float line_offset = 400.f - line_height / 2.f;

        // Render walls
        SDL_Rect src = {
            .x = ((float)((collision_type == COLLISION_HORIZONTAL ? endp.x : endp.y) % p->map->tile_size) / (float)p->map->tile_size) * p->image_size.x,
            .y = 0,
            .w = 1,
            .h = p->image_size.y
        };

        SDL_Rect dst = { .x = x_pos, .y = (int)line_offset, .w = 1, .h = (int)line_height };
        SDL_RenderCopy(p->rend, p->tile_texture, &src, &dst);


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
            int ray_length_entity = player_cast_ray_entity(p->player, i, p->entities, p->entities_size, ignored_entities, ignored_entities_size, &intersection, &entity_hit);

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

        // Render entities
        for (int j = 0; j < entity_ray_lengths_size; ++j)
        {
            if (entity_ray_lengths[j] < ray_length_wall && entity_ray_lengths[j] != -1)
            {
                src.x = (intersections[j] / ignored_entities[j]->width) * ignored_entities[j]->sprite_size.x;

                dist = entity_ray_lengths[j] * cosf(angle);
                line_height = (25.f * 800.f) / dist;

                line_offset = 400.f;

                dst.y = line_offset;
                dst.h = line_height;

                SDL_RenderCopy(p->rend, ignored_entities[j]->sprite, &src, &dst);
            }
        }

        free(ignored_entities);
        free(entity_ray_lengths);
        free(intersections);

        x_pos += 1.f;
    }
}


void prog_render_map(struct Prog* p)
{
    SDL_SetRenderDrawColor(p->rend, 180, 180, 0, 255);

    for (int i = 0; i < strlen(p->map->layout); ++i)
    {
        if (p->map->layout[i] == '#')
        {
            SDL_Rect rect = {
                .x = (i % p->map->size.x) * p->map->tile_size,
                .y = ((i - (i % p->map->size.x)) / p->map->size.x) * p->map->tile_size,
                .w = 50,
                .h = 50
            };

            SDL_RenderFillRect(p->rend, &rect);
        }
    }
}


void prog_render_gun(struct Prog* p)
{
    SDL_Texture* tex = p->player->shooting ? p->shot_texture : p->gun_texture;

    static SDL_Rect rect = { .x = 500, .y = 500 };
    static bool finished_reloading = false;

    if (p->player->reloading)
    {
        if (!finished_reloading)
            rect.y += 20;
        else
            rect.y -= 20;

        if (!finished_reloading && rect.y >= 2000)
        {
            p->player->bullets = 20;
            finished_reloading = true;
        }

        if (finished_reloading && rect.y <= 500)
        {
            p->player->reloading = false;
            finished_reloading = false;
        }
    }

    SDL_QueryTexture(tex, 0, 0, &rect.w, &rect.h);

    SDL_RenderCopy(p->rend, tex, 0, &rect);
}


void prog_add_entity(struct Prog* p)
{
    ++p->entities_size;
    p->entities = realloc(p->entities, sizeof(struct Entity*) * p->entities_size);

    struct Entity* e = entity_init((SDL_FPoint){ rand() % (p->map->size.x * p->map->tile_size), rand() % (p->map->size.y * p->map->tile_size) }, p->rend, "res/goomba.png");
    SDL_Point grid_pos = {
        .x = (e->pos.x - ((int)e->pos.x % p->map->tile_size)) / p->map->tile_size,
        .y = (e->pos.y - ((int)e->pos.y % p->map->tile_size)) / p->map->tile_size
    };

    SDL_Point diff = { .x = e->pos.x - p->player->rect.x, .y = e->pos.y - p->player->rect.y };
    float distance_to_player = sqrtf(diff.x * diff.x + diff.y * diff.y);

    while (p->map->layout[grid_pos.y * p->map->size.x + grid_pos.x] == '#' || distance_to_player < 300)
    {
        e->pos.x = rand() % (p->map->size.x * p->map->tile_size);
        e->pos.y = rand() % (p->map->size.y * p->map->tile_size);

        grid_pos.x = (e->pos.x - ((int)e->pos.x % p->map->tile_size)) / p->map->tile_size;
        grid_pos.y = (e->pos.y - ((int)e->pos.y % p->map->tile_size)) / p->map->tile_size;

        diff.x = e->pos.x - p->player->rect.x;
        diff.y = e->pos.y - p->player->rect.y;

        distance_to_player = sqrtf(diff.x * diff.x + diff.y * diff.y);
    }

    p->entities[p->entities_size - 1] = e;
}


void prog_remove_entity(struct Prog* p, struct Entity* entity)
{
    struct Entity** entities = malloc(sizeof(struct Entity*) * (p->entities_size - 1));
    int index_offset = 0;

    for (int i = 0; i < p->entities_size; ++i)
    {
        if (p->entities[i] == entity)
        {
            index_offset = -1;
            continue;
        }

        entities[i + index_offset] = p->entities[i];
    }

    free(entity);
    free(p->entities);
    p->entities = entities;
    --p->entities_size;
}

