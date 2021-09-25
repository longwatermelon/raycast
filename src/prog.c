#include "prog.h"
#include "common.h"
#include "entity.h"
#include "player.h"
#include <SDL_image.h>


struct Prog* prog_init()
{
    struct Prog* p = malloc(sizeof(struct Prog));
    p->running = true;

    p->window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, SDL_WINDOW_SHOWN);
    p->rend = SDL_CreateRenderer(p->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    p->player = player_init((SDL_Point){ 300, 320 }, M_PI);
    p->map = map_init("map", (SDL_Point){ 16, 16 }, 50);

    p->entities = malloc(0);
    p->entities_size = 0;

    p->entities = realloc(p->entities, sizeof(struct Entity*));
    p->entities[0] = entity_init((SDL_FPoint){ 4 * p->map->tile_size + 10, 3 * p->map->tile_size - 5 }, p->rend, "deezn.png");
    p->entities_size = 1;

    p->tile_texture = IMG_LoadTexture(p->rend, "deez.png");
    SDL_QueryTexture(p->tile_texture, 0, 0, &p->image_size.x, &p->image_size.y);

    return p;
}


void prog_cleanup(struct Prog* p)
{
    SDL_DestroyTexture(p->tile_texture);
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

        SDL_RenderClear(p->rend);

        SDL_SetRenderDrawColor(p->rend, 255, 0, 0, 255);
        float x_pos = 0.f;

        for (float i = p->player->angle + M_PI / 6.f; i > p->player->angle - M_PI / 6.f; i -= 0.0013f) // Cast 800 rays
        {
            int collision_type;
            SDL_Point endp = player_cast_ray(p->player, i, p->map, p->entities, p->entities_size, &collision_type);

            int ray_length_wall = sqrtf((endp.x - p->player->rect.x) * (endp.x - p->player->rect.x) + (endp.y - p->player->rect.y) * (endp.y - p->player->rect.y));

            float intersection;
            struct Entity* entity_hit;
            int ray_length_entity = player_cast_ray_entity(p->player, i, p->entities, p->entities_size, &intersection, &entity_hit);

            float angle = common_restrict_angle(p->player->angle - i);

            // Adjust for fisheye effect
            float dist = ray_length_wall * cosf(angle);
            float line_height = (p->map->tile_size * 800.f) / dist;

            /* if (line_height > 800.f) */
            /*     line_height = 800.f; */

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

            // Render entities
            if (ray_length_entity < ray_length_wall && ray_length_entity != -1)
            {
                src.x = (intersection / 10.f) * entity_hit->sprite_size.x;
                
                dist = ray_length_entity * cosf(angle);
                line_height = (25.f * 800.f) / dist;

                // line_offset = 400.f - line_height / 2.f + line_height / 2.f
                line_offset = 400.f;

                dst.y = line_offset;
                dst.h = line_height;

                SDL_RenderCopy(p->rend, entity_hit->sprite, &src, &dst);
            }

            x_pos += 1.f;
        }

        /* prog_render_map(p); */
        /* player_render(p->player, p->rend, p->map, p->entities, p->entities_size); */

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
                .w = p->map->tile_size,
                .h = p->map->tile_size
            };

            SDL_RenderFillRect(p->rend, &rect);
        }
    }
}

