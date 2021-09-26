#include "prog.h"
#include "common.h"
#include "entity.h"
#include "player.h"
#include "events.h"
#include "render.h"
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
        events_base(p, &evt);

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

        render_3d_all(p);
        /* prog_render_3d(p); */
        prog_render_gun(p);

        common_display_statistic(p->rend, p->font, "Bullets: ", p->player->bullets, (SDL_Point){ 20, 20 });
        common_display_statistic(p->rend, p->font, "Enemies alive: ", p->entities_size, (SDL_Point){ 20, 40 });

        SDL_Rect crosshair = { .x = 400 - 2, .y = 400 - 2, .w = 4, .h = 4 };
        SDL_SetRenderDrawColor(p->rend, 255, 0, 0, 255);
        SDL_RenderFillRect(p->rend, &crosshair);

        SDL_SetRenderDrawColor(p->rend, 0, 0, 0, 255);
        SDL_RenderPresent(p->rend);
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

