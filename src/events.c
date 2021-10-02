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
            events_keydown(p, evt);
            break;
        case SDL_MOUSEMOTION:
        {
            p->player->angle -= 0.002f * evt->motion.xrel;
        } break;
        case SDL_MOUSEBUTTONDOWN:
            events_mouse_down(p, evt);
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
            if (!p->player->alive)
                break;

            struct Entity* hit = player_shoot(p->player, p->entities, p->entities_size, p->map);
            if (hit)
            {
                audio_play_sound("res/sfx/scream.wav");
                ++p->player->enemies_killed;
                prog_remove_entity(p, hit);
            }
        } break;
        case SDLK_r:
        {
            if (p->player->reloading)
                break;

            audio_play_sound("res/sfx/reload.wav");
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

    switch (evt->key.keysym.sym)
    {
    case SDLK_ESCAPE:
        SDL_SetRelativeMouseMode(SDL_FALSE);
        break;
    }
}


void events_mouse_down(struct Prog* p, SDL_Event* evt)
{
    SDL_SetRelativeMouseMode(SDL_TRUE);

    if (evt->button.button == SDL_BUTTON_LEFT)
        events_mouse_down_left(p, evt);
    else if (evt->button.button == SDL_BUTTON_RIGHT)
        events_mouse_down_right(p, evt);
}


void events_mouse_down_left(struct Prog* p, SDL_Event* evt)
{
    if (!p->player->alive)
        return;

    struct Entity* hit = player_shoot(p->player, p->entities, p->entities_size, p->map);
    if (hit)
    {
        audio_play_sound("res/sfx/scream.wav");
        ++p->player->enemies_killed;
        prog_remove_entity(p, hit);
    }
}


void events_mouse_down_right(struct Prog* p, SDL_Event* evt)
{
}

