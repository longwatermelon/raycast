#include "events.h"
#include "prog.h"
#include "audio.h"
#include <time.h>


void events_base(struct Prog* p, SDL_Event* evt)
{
    /* while (SDL_PollEvent(evt)) */
    /* { */
        

    while (SDL_PollEvent(evt))
    {
        switch (evt->type)
        {
        case SDL_QUIT:
            p->running = false;
            break;
        case SDL_KEYDOWN:
            events_keydown(p, evt);
            break;
        }
    }
   
    events_no_delay(p);
}


void events_no_delay(struct Prog* p)
{
    float player_speed = 2.f;

    const Uint8* keystates = SDL_GetKeyboardState(0);

    if (p->player->alive)
    {
        if (keystates[SDL_SCANCODE_W])
            player_move(p->player, p->map, player_speed * cosf(p->player->angle), player_speed * -sinf(p->player->angle));

        if (keystates[SDL_SCANCODE_S])
            player_move(p->player, p->map, -player_speed * cosf(p->player->angle), -player_speed * -sinf(p->player->angle));

        if (keystates[SDL_SCANCODE_A])
            player_move(p->player, p->map, player_speed / 2.f * cosf(p->player->angle + M_PI / 2.f), player_speed / 2.f * -sinf(p->player->angle + M_PI / 2.f));

        if (keystates[SDL_SCANCODE_D])
            player_move(p->player, p->map, player_speed / 2.f * cosf(p->player->angle - M_PI / 2.f), player_speed / 2.f * -sinf(p->player->angle - M_PI / 2.f));

        if (keystates[SDL_SCANCODE_RIGHT])
            p->player->angle -= 0.03f;

        if (keystates[SDL_SCANCODE_LEFT])
            p->player->angle += 0.03f;
    }
}


void events_keydown(struct Prog* p, SDL_Event* evt)
{
    if (p->player->alive)
    {
        switch (evt->key.keysym.sym)
        {
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
            if (p->player->reloading)
                break;

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
            p->restart = true;
            p->running = false;
            break;
        }
    }
}

