#include "prog.h"
#include "common.h"
#include "entity.h"
#include "player.h"
#include "events.h"
#include "render.h"
#include "audio.h"
#include <time.h>
#include <SDL_image.h>


struct Prog* prog_init(SDL_Window* window, SDL_Renderer* rend)
{
    struct Prog* p = malloc(sizeof(struct Prog));
    p->running = true;

    p->window = window;
    p->rend = rend;

    p->font = TTF_OpenFont("res/font.ttf", 16);

    p->player = player_init((SDL_Point){ 300, 320 }, M_PI);
    p->map = map_init("map", (SDL_Point){ 32, 32 }, 50);

    p->entities = malloc(0);
    p->entities_size = 0;

    p->tile_texture = IMG_LoadTexture(p->rend, "res/wall.png");
    SDL_QueryTexture(p->tile_texture, 0, 0, &p->image_size.x, &p->image_size.y);

    p->gun_texture = IMG_LoadTexture(p->rend, "res/gun.png");
    p->shot_texture = IMG_LoadTexture(p->rend, "res/gun_shoot.png");

    p->restart = false;

    p->enemies_killed = 0;

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

    free(p);
}


void prog_mainloop(struct Prog* p)
{
    SDL_Event evt;

    while (p->running)
    {
        events_base(p, &evt);
        audio_stop_finished_sounds();

        player_move(p->player, p->map);

        for (int i = 0; i < p->entities_size; ++i)
        {
            if (p->player->alive && p->entities[i]->type == ENTITY_ENEMY)
                entity_move_towards_player(p->entities[i], p->player, p->map);

            SDL_FPoint diff = {
                .x = p->player->rect.x - p->entities[i]->pos.x,
                .y = p->player->rect.y - p->entities[i]->pos.y
            };

            float distance = sqrtf(diff.x * diff.x + diff.y * diff.y);

            if (distance <= p->entities[i]->width / 2.f)
            {
                if (p->entities[i]->type == ENTITY_ENEMY)
                {
                    p->player->alive = false;
                    p->player->speed = 0.f;
                    p->player->angle_change = 0.f;
                    break;
                }
                else if (p->entities[i]->type == ENTITY_AMMO)
                {
                    audio_play_sound("res/ammo.wav");
                    p->player->bullets += 30;
                    prog_remove_entity(p, p->entities[i]);
                    break;
                }
            }
        }

        if (p->player->alive)
        {
            if (p->player->shooting)
            {
                if ((float)(clock() - p->player->last_shot_time) / CLOCKS_PER_SEC >= .01f)
                {
                    p->player->shooting = false;
                }
            }

            if (p->entities_size < 15)
            {
                if (rand() % 2000 > 1985)
                    prog_spawn_entity(p, ENTITY_ENEMY, "res/goomba.png");

                if (rand() % 2000 > 1996)
                    prog_spawn_entity(p, ENTITY_AMMO, "res/deez.png");
            }
        }
        
        SDL_RenderClear(p->rend);

        render_3d_all(p);
        prog_render_gun(p);

        common_display_statistic(p->rend, p->font, "Bullets loaded: ", p->player->bullets_loaded, (SDL_Point){ 20, 20 });
        common_display_statistic(p->rend, p->font, "Unused bullets: ", p->player->bullets, (SDL_Point){ 20, 40 });
        common_display_statistic(p->rend, p->font, "Enemies killed: ", p->enemies_killed, (SDL_Point){ 20, 60 });

        SDL_Rect crosshair = { .x = 400 - 2, .y = 400 - 2, .w = 4, .h = 4 };
        SDL_SetRenderDrawColor(p->rend, 255, 0, 0, 255);
        SDL_RenderFillRect(p->rend, &crosshair);

        if (!p->player->alive)
        {
            SDL_SetRenderDrawBlendMode(p->rend, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(p->rend, 0, 0, 0, 200);
            SDL_RenderFillRect(p->rend, 0);
            SDL_SetRenderDrawBlendMode(p->rend, SDL_BLENDMODE_NONE);

            SDL_Texture* game_over_tex = common_render_text(p->rend, p->font, "Game over, press r to restart");
            SDL_Rect tmp = { .x = 300, .y = 380 };
            SDL_QueryTexture(game_over_tex, 0, 0, &tmp.w, &tmp.h);
            SDL_RenderCopy(p->rend, game_over_tex, 0, &tmp);

            SDL_DestroyTexture(game_over_tex);
        }

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
            p->player->bullets += p->player->bullets_loaded;
            p->player->bullets -= 20;
            p->player->bullets_loaded = 20;

            if (p->player->bullets < 0)
            {
                p->player->bullets_loaded += p->player->bullets;
                p->player->bullets = 0;
            }

            finished_reloading = true;
        }

        if (finished_reloading && rect.y <= 500)
        {
            p->player->reloading = false;
            finished_reloading = false;
        }
    }

    if (!p->player->reloading)
    {
        rect.y = 500;
    }

    SDL_QueryTexture(tex, 0, 0, &rect.w, &rect.h);

    SDL_RenderCopy(p->rend, tex, 0, &rect);
}


void prog_add_entity(struct Prog* p, struct Entity* entity)
{
    ++p->entities_size;
    p->entities = realloc(p->entities, sizeof(struct Entity*) * p->entities_size);
    p->entities[p->entities_size - 1] = entity;
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


void prog_spawn_entity(struct Prog* p, int type, const char* sprite_path)
{
    struct Entity* e = entity_init(type, (SDL_FPoint){ rand() % (p->map->size.x * p->map->tile_size), rand() % (p->map->size.y * p->map->tile_size) }, p->rend, sprite_path);

    while (true)
    {
        e->pos = map_get_random_empty_spot(p->map);
        SDL_Point diff = { .x = e->pos.x - p->player->rect.x, .y = e->pos.y - p->player->rect.y };
        float distance_to_player = sqrtf(diff.x * diff.x + diff.y * diff.y);

        if (distance_to_player > 300)
            break;
    }

    prog_add_entity(p, e);
}

