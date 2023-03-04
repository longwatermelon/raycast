#include "prog.h"
#include "common.h"
#include "entity.h"
#include "player.h"
#include "events.h"
#include "render.h"
#include "audio.h"
#include <time.h>
#include <SDL2/SDL_image.h>


struct Prog *prog_init(SDL_Window *window, SDL_Renderer *rend)
{
    struct Prog *self = malloc(sizeof(struct Prog));
    self->running = true;

    self->window = window;
    self->rend = rend;

    self->font = TTF_OpenFont("res/gfx/font.ttf", 16);

    if (!self->font)
    {
        fprintf(stderr, "Couldn't open font\n");
        return 0;
    }

    self->map = map_init("res/map", (SDL_Point){ 32, 32 }, 50);

    if (!self->map)
        return 0;

    SDL_FPoint pos = map_get_random_empty_spot(self->map);
    self->player = player_init((SDL_Point){ .x = (int)pos.x,  .y = (int)pos.y }, M_PI, self->rend);

    if (!self->player)
        return 0;

    self->entities = malloc(0);
    self->entities_size = 0;

    self->tile_texture = IMG_LoadTexture(self->rend, "res/gfx/wall.png");
    SDL_QueryTexture(self->tile_texture, 0, 0, &self->image_size.x, &self->image_size.y);

    if (!self->tile_texture)
    {
        fprintf(stderr, "Wall textures are missing\n");
        return 0;
    }

    self->crosshair_texture = IMG_LoadTexture(self->rend, "res/gfx/crosshair.png");

    if (!self->crosshair_texture)
    {
        fprintf(stderr, "Missing crosshair texture\n");
        return 0;
    }

    self->game_over = false;
    self->restart = false;
    self->win = false;

    self->nuts_collected = 0;

    self->render_entities_over_walls = false;
    self->adjust_fisheye = true;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    return self;
}


void prog_cleanup(struct Prog *self)
{
    SDL_SetRelativeMouseMode(SDL_FALSE);

    SDL_DestroyTexture(self->tile_texture);
    SDL_DestroyTexture(self->crosshair_texture);

    TTF_CloseFont(self->font);

    player_cleanup(self->player);
    map_cleanup(self->map);

    for (int i = 0; i < self->entities_size; ++i)
        entity_cleanup(self->entities[i]);

    free(self->entities);

    free(self);
}


void prog_mainloop(struct Prog *self)
{
    SDL_Event evt;

#ifndef __EMSCRIPTEN__
    while (self->running)
#endif
    {
        events_base(self, &evt);
        audio_stop_finished_sounds();

        prog_handle_entity_interaction(self);

        if (!self->game_over)
        {
            if (self->player->shooting)
            {
                struct timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);

                if (common_time_diff(self->player->last_shot_time, now) >= 0.05f)
                {
                    self->player->shooting = false;
                }
            }

            if (rand() % 2000 > 1930 && self->entities_size < MAX_ENTITIES)
                prog_spawn_entity(self, ENTITY_ENEMY, "res/gfx/shrek.png");

            if (rand() % 2000 > 1995 && self->entities_size < MAX_ENTITIES)
                prog_spawn_entity(self, ENTITY_AMMO, "res/gfx/ammo.png");

            if (rand() % 2000 > 1990 && self->entities_size < MAX_ENTITIES)
                prog_spawn_entity(self, ENTITY_GRENADE, "res/gfx/grenade.png");

            // Spawning nuts
            bool nuts_exist = false;

            for (int i = 0; i < self->entities_size; ++i)
            {
                if (self->entities[i]->type == ENTITY_NUTS)
                {
                    nuts_exist = true;
                }
            }

            if (!nuts_exist)
                prog_spawn_entity(self, ENTITY_NUTS, "res/gfx/deez.png");
        }

        if (self->nuts_collected >= 5)
        {
            self->win = true;
            self->game_over = true;
        }

        if (self->player->health <= 0)
        {
            self->game_over = true;
        }

        player_execute_mode(self->player);
        player_advance_animations(self->player);

        SDL_RenderClear(self->rend);

        prog_render_all(self);

        SDL_SetRenderDrawColor(self->rend, 0, 0, 0, 255);
        SDL_RenderPresent(self->rend);
    }
}


void prog_handle_entity_interaction(struct Prog *self)
{
    for (int i = 0; i < self->entities_size; ++i)
    {
        if (self->entities[i]->enemy_dead)
        {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);

            if (common_time_diff(self->entities[i]->enemy_death_time, now) >= 1.f)
            {
                prog_remove_entity(self, self->entities[i]);
                --i;
            }

            continue;
        }

        if (!self->game_over && self->entities[i]->type == ENTITY_ENEMY)
            entity_move_towards_player(self->entities[i], self->rend, self->player, self->map);

        SDL_FPoint diff = {
            .x = self->player->pos.x - self->entities[i]->pos.x,
            .y = self->player->pos.y - self->entities[i]->pos.y
        };

        float distance = sqrtf(diff.x * diff.x + diff.y * diff.y);

        if (distance <= self->entities[i]->width / 2.f)
        {
            if (self->entities[i]->type == ENTITY_ENEMY)
            {
                if (!self->game_over)
                {
                    struct timespec now;
                    clock_gettime(CLOCK_MONOTONIC, &now);

                    if (common_time_diff(self->player->last_hurt_time, now) > 1.f)
                    {
                        self->player->last_hurt_time = now;
                        --self->player->health;
                    }

                    self->player->angle_change = 0.f;
                    break;
                }
            }
            else if (self->entities[i]->type == ENTITY_AMMO)
            {
                audio_play_sound("res/sfx/ammo.wav");
                self->player->bullets += 16;
                prog_remove_entity(self, self->entities[i]);
                break;
            }
            else if (self->entities[i]->type == ENTITY_NUTS)
            {
                audio_play_sound("res/sfx/nuts.wav");
                ++self->nuts_collected;
                prog_remove_entity(self, self->entities[i]);
                break;
            }
            else if (self->entities[i]->type == ENTITY_GRENADE)
            {
                audio_play_sound("res/sfx/ammo.wav");
                ++self->player->grenades;
                prog_remove_entity(self, self->entities[i]);
                break;
            }
        }
    }
}


void prog_render_all(struct Prog *self)
{
    render_3d_all(self);
    player_render_weapon(self->player, self->rend);

    common_display_statistic(self->rend, self->font, "Health: ", self->player->health, (SDL_Point){ 20, 20 });
    common_display_statistic(self->rend, self->font, "Nuts collected: ", self->nuts_collected, (SDL_Point){ 20, 40 });
    common_display_statistic(self->rend, self->font, "Grenades: ", self->player->grenades, (SDL_Point){ 20, 60 });

    if (self->player->weapon == WEAPON_GUN)
    {
        common_display_statistic(self->rend, self->font, "Bullets loaded: ", self->player->bullets_loaded, (SDL_Point){ 20, 740 });
        common_display_statistic(self->rend, self->font, "Unused bullets: ", self->player->bullets, (SDL_Point){ 20, 760 });
    }

    if (self->player->mode_data.mode == PLAYER_MODE_GRAPPLING)
    {
        SDL_Texture *tex = common_render_text(self->rend, self->font, "[Grappling hook]");
        SDL_Rect tmp = { .x = 640, .y = 20 };
        SDL_QueryTexture(tex, 0, 0, &tmp.w, &tmp.h);
        SDL_RenderCopy(self->rend, tex, 0, &tmp);
        SDL_DestroyTexture(tex);
    }

    if (self->player->weapon == WEAPON_GUN)
    {
        SDL_Rect crosshair = { .x = 400 - 8, .y = 400 - 8, .w = 16, .h = 16 };
        SDL_RenderCopy(self->rend, self->crosshair_texture, 0, &crosshair);
    }

    if (self->player->health != 3)
    {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        if ((self->game_over && !self->win) || common_time_diff(self->player->last_hurt_time, now) < 1.f)
        {
            SDL_SetRenderDrawBlendMode(self->rend, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(self->rend, 255, 0, 0, 150);
            SDL_RenderFillRect(self->rend, 0);
            SDL_SetRenderDrawBlendMode(self->rend, SDL_BLENDMODE_NONE);
        }
    }

#ifdef CHEATS_ON
    {
        SDL_Texture *cheats_notif = common_render_text(self->rend, self->font, "[Cheats enabled]");
        SDL_Rect tmp = { .x = 20, .y = 80 };
        SDL_QueryTexture(cheats_notif, 0, 0, &tmp.w, &tmp.h);
        SDL_RenderCopy(self->rend, cheats_notif, 0, &tmp);
        SDL_DestroyTexture(cheats_notif);
    }
#endif

    if (self->game_over)
    {
        SDL_SetRenderDrawBlendMode(self->rend, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(self->rend, 0, 0, 0, 200);
        SDL_RenderFillRect(self->rend, 0);
        SDL_SetRenderDrawBlendMode(self->rend, SDL_BLENDMODE_NONE);

        if (self->win)
        {
            SDL_Texture *win_tex = common_render_text(self->rend, self->font, "All the nuts were successfully collected, press r to restart");
            SDL_Rect tmp = { .x = 125, .y = 380 };
            SDL_QueryTexture(win_tex, 0, 0, &tmp.w, &tmp.h);
            SDL_RenderCopy(self->rend, win_tex, 0, &tmp);

            SDL_DestroyTexture(win_tex);
        }
        else
        {
            SDL_Texture *game_over_tex = common_render_text(self->rend, self->font, "Game over, press r to restart");
            SDL_Rect tmp = { .x = 275, .y = 380 };
            SDL_QueryTexture(game_over_tex, 0, 0, &tmp.w, &tmp.h);
            SDL_RenderCopy(self->rend, game_over_tex, 0, &tmp);

            SDL_DestroyTexture(game_over_tex);
        }
    }
}


void prog_add_entity(struct Prog *self, struct Entity *entity)
{
    ++self->entities_size;
    self->entities = realloc(self->entities, sizeof(struct Entity*) * self->entities_size);
    self->entities[self->entities_size - 1] = entity;
}


void prog_remove_entity(struct Prog *self, struct Entity *entity)
{
    struct Entity **entities = malloc(sizeof(struct Entity*) * (self->entities_size - 1));
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


void prog_spawn_entity(struct Prog *self, int type, const char *sprite_path)
{
    struct Entity *e = entity_init(type, (SDL_FPoint){ rand() % (self->map->size.x * self->map->tile_size), rand() % (self->map->size.y * self->map->tile_size) }, self->rend, sprite_path);

    while (true)
    {
        e->pos = map_get_random_empty_spot(self->map);
        SDL_Point diff = { .x = e->pos.x - self->player->pos.x, .y = e->pos.y - self->player->pos.y };
        float distance_to_player = sqrtf(diff.x * diff.x + diff.y * diff.y);

        if (distance_to_player > 300)
            break;
    }

    prog_add_entity(self, e);
}


int prog_entity_count(struct Prog *self, int type)
{
    int count = 0;

    for (int i = 0; i < self->entities_size; ++i)
    {
        if (self->entities[i]->type == type)
            ++count;
    }

    return count;
}

void prog_blast(struct Prog *self)
{
    if (self->player->grenades == 0)
        return;

    --self->player->grenades;
    audio_play_sound("res/sfx/grenade.wav");
    render_flash();
    for (size_t i = 0; i < self->entities_size; ++i)
    {
        SDL_FPoint diff = {
            self->player->pos.x - self->entities[i]->pos.x,
            self->player->pos.y - self->entities[i]->pos.y
        };

        float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
        if (dist > 300.f)
            continue;

        self->entities[i]->blasted = true;
        self->entities[i]->blast_dir = (SDL_FPoint){
            -diff.x / dist * 20.f,
            -diff.y / dist * 20.f
        };
    }
}
