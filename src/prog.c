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
    struct Prog* self = malloc(sizeof(struct Prog));
    self->running = true;

    self->window = window;
    self->rend = rend;

    self->font = TTF_OpenFont("res/font.ttf", 16);

    self->map = map_init("map", (SDL_Point){ 32, 32 }, 50);

    SDL_FPoint pos = map_get_random_empty_spot(self->map);
    self->player = player_init((SDL_Point){ .x = (int)pos.x,  .y = (int)pos.y }, M_PI);

    self->entities = malloc(0);
    self->entities_size = 0;

    self->tile_texture = IMG_LoadTexture(self->rend, "res/wall.png");
    SDL_QueryTexture(self->tile_texture, 0, 0, &self->image_size.x, &self->image_size.y);

    self->gun_texture = IMG_LoadTexture(self->rend, "res/gun.png");
    self->shot_texture = IMG_LoadTexture(self->rend, "res/gun_shoot.png");

    self->restart = false;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    return self;
}


void prog_cleanup(struct Prog* self)
{
    SDL_SetRelativeMouseMode(SDL_FALSE);

    SDL_DestroyTexture(self->tile_texture);
    SDL_DestroyTexture(self->gun_texture);

    TTF_CloseFont(self->font);

    player_cleanup(self->player);
    map_cleanup(self->map);

    for (int i = 0; i < self->entities_size; ++i)
        entity_cleanup(self->entities[i]);

    free(self->entities);

    free(self);
}


void prog_mainloop(struct Prog* self)
{
    SDL_Event evt;

    while (self->running)
    {
        events_base(self, &evt);
        audio_stop_finished_sounds();

        for (int i = 0; i < self->entities_size; ++i)
        {
            if (self->player->alive && self->entities[i]->type == ENTITY_ENEMY)
                entity_move_towards_player(self->entities[i], self->player, self->map);

            SDL_FPoint diff = {
                .x = self->player->rect.x - self->entities[i]->pos.x,
                .y = self->player->rect.y - self->entities[i]->pos.y
            };

            float distance = sqrtf(diff.x * diff.x + diff.y * diff.y);

            if (distance <= self->entities[i]->width / 2.f)
            {
                if (self->entities[i]->type == ENTITY_ENEMY)
                {
                    self->player->alive = false;
                    self->player->angle_change = 0.f;
                    break;
                }
                else if (self->entities[i]->type == ENTITY_AMMO)
                {
                    audio_play_sound("res/ammo.wav");
                    self->player->bullets += 16;
                    prog_remove_entity(self, self->entities[i]);
                    break;
                }
            }
        }

        if (self->player->alive)
        {
            if (self->player->shooting)
            {
                if ((float)(clock() - self->player->last_shot_time) / CLOCKS_PER_SEC >= .01f)
                {
                    self->player->shooting = false;
                }
            }

            if (self->entities_size < 15)
            {
                if (rand() % 2000 > 1930)
                    prog_spawn_entity(self, ENTITY_ENEMY, "res/shrek.png");

                if (rand() % 2000 > 1980)
                    prog_spawn_entity(self, ENTITY_AMMO, "res/deez.png");
            }
        }
        
        SDL_RenderClear(self->rend);

        render_3d_all(self);
        prog_render_gun(self);

        common_display_statistic(self->rend, self->font, "Bullets loaded: ", self->player->bullets_loaded, (SDL_Point){ 20, 20 });
        common_display_statistic(self->rend, self->font, "Unused bullets: ", self->player->bullets, (SDL_Point){ 20, 40 });
        common_display_statistic(self->rend, self->font, "Enemies killed: ", self->player->enemies_killed, (SDL_Point){ 20, 60 });

        SDL_Rect crosshair = { .x = 400 - 2, .y = 400 - 2, .w = 4, .h = 4 };
        SDL_SetRenderDrawColor(self->rend, 255, 0, 0, 255);
        SDL_RenderFillRect(self->rend, &crosshair);

        if (!self->player->alive)
        {
            SDL_SetRenderDrawBlendMode(self->rend, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(self->rend, 0, 0, 0, 200);
            SDL_RenderFillRect(self->rend, 0);
            SDL_SetRenderDrawBlendMode(self->rend, SDL_BLENDMODE_NONE);

            SDL_Texture* game_over_tex = common_render_text(self->rend, self->font, "Game over, press r to restart");
            SDL_Rect tmp = { .x = 300, .y = 380 };
            SDL_QueryTexture(game_over_tex, 0, 0, &tmp.w, &tmp.h);
            SDL_RenderCopy(self->rend, game_over_tex, 0, &tmp);

            SDL_DestroyTexture(game_over_tex);
        }

        SDL_SetRenderDrawColor(self->rend, 0, 0, 0, 255);
        SDL_RenderPresent(self->rend);
    }
}


void prog_render_map(struct Prog* self)
{
    SDL_SetRenderDrawColor(self->rend, 180, 180, 0, 255);

    for (int i = 0; i < strlen(self->map->layout); ++i)
    {
        if (self->map->layout[i] == '#')
        {
            SDL_Rect rect = {
                .x = (i % self->map->size.x) * self->map->tile_size,
                .y = ((i - (i % self->map->size.x)) / self->map->size.x) * self->map->tile_size,
                .w = 50,
                .h = 50
            };

            SDL_RenderFillRect(self->rend, &rect);
        }
    }
}


void prog_render_gun(struct Prog* self)
{
    SDL_Texture* tex = self->player->shooting ? self->shot_texture : self->gun_texture;

    static SDL_Rect rect = { .x = 500, .y = 500 };
    static bool finished_reloading = false;

    if (self->player->reloading)
    {
        if (!finished_reloading)
            rect.y += 20;
        else
            rect.y -= 20;

        if (!finished_reloading && rect.y >= 2000)
        {
            self->player->bullets += self->player->bullets_loaded;
            self->player->bullets -= 16;
            self->player->bullets_loaded = 16;

            if (self->player->bullets < 0)
            {
                self->player->bullets_loaded += self->player->bullets;
                self->player->bullets = 0;
            }

            finished_reloading = true;
        }

        if (finished_reloading && rect.y <= 500)
        {
            self->player->reloading = false;
            finished_reloading = false;
        }
    }

    if (!self->player->reloading)
    {
        rect.y = 500;
        finished_reloading = false;
    }

    SDL_QueryTexture(tex, 0, 0, &rect.w, &rect.h);

    SDL_RenderCopy(self->rend, tex, 0, &rect);
}


void prog_add_entity(struct Prog* self, struct Entity* entity)
{
    ++self->entities_size;
    self->entities = realloc(self->entities, sizeof(struct Entity*) * self->entities_size);
    self->entities[self->entities_size - 1] = entity;
}


void prog_remove_entity(struct Prog* self, struct Entity* entity)
{
    struct Entity** entities = malloc(sizeof(struct Entity*) * (self->entities_size - 1));
    int index_offset = 0;

    for (int i = 0; i < self->entities_size; ++i)
    {
        if (self->entities[i] == entity)
        {
            index_offset = -1;
            continue;
        }

        entities[i + index_offset] = self->entities[i];
    }

    free(entity);
    free(self->entities);
    self->entities = entities;
    --self->entities_size;
}


void prog_spawn_entity(struct Prog* self, int type, const char* sprite_path)
{
    struct Entity* e = entity_init(type, (SDL_FPoint){ rand() % (self->map->size.x * self->map->tile_size), rand() % (self->map->size.y * self->map->tile_size) }, self->rend, sprite_path);

    while (true)
    {
        e->pos = map_get_random_empty_spot(self->map);
        SDL_Point diff = { .x = e->pos.x - self->player->rect.x, .y = e->pos.y - self->player->rect.y };
        float distance_to_player = sqrtf(diff.x * diff.x + diff.y * diff.y);

        if (distance_to_player > 300)
            break;
    }

    prog_add_entity(self, e);
}

