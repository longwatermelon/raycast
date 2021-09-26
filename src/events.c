#include "events.h"
#include "prog.h"
#include "audio.h"
#include <time.h>


void events_base(struct Prog* p, SDL_Event* evt)
{
    while (SDL_PollEvent(evt))
    {
        switch (evt->type)
        {
        case SDL_QUIT:
            p->running = false;
            break;
        case SDL_KEYDOWN:
        {
            events_keydown(p, evt);
        } break;
        case SDL_KEYUP:
        {
            events_keyup(p, evt);
        } break;
        }
    }
}


void events_keydown(struct Prog* p, SDL_Event* evt)
{
    float player_speed = 2.f;

    if (p->player->alive)
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
        case SDLK_SPACE:
        {
            if (p->player->bullets_loaded <= 0 || p->player->reloading)
                break;

            p->player->shooting = true;
            p->player->last_shot_time = clock();
            --p->player->bullets_loaded;

            audio_play_sound("res/gunshot.wav");

            float intersection;
            struct Entity* entity = 0;
            int entity_dist = player_cast_ray_entity(p->player, p->player->angle, p->entities, p->entities_size, 0, 0, ENTITY_ENEMY, &intersection, &entity);

            int collision_type;
            SDL_Point wall_vector = player_cast_ray(p->player, p->player->angle, p->map, p->entities, p->entities_size, &collision_type);
            SDL_Point diff = { .x = wall_vector.x - p->player->rect.x, .y = wall_vector.y - p->player->rect.y };
            int wall_dist = sqrtf(diff.x * diff.x + diff.y * diff.y);

            if (entity_dist != -1 && entity_dist < wall_dist)
            {
                audio_play_sound("res/scream.wav");
                ++p->enemies_killed;
                prog_remove_entity(p, entity);
            }
        } break;
        case SDLK_r:
        {
            audio_play_sound("res/reload.wav");
            p->player->reloading = true;
        } break;
        }
    }
    else
    {
        switch (evt->key.keysym.sym)
        {
        case SDLK_r:
        {
            p->running = false;
            p->restart = true;
        } break;
        }
    }
}


void events_keyup(struct Prog* p, SDL_Event* evt)
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
}

