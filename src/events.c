#include "events.h"
#include "prog.h"
#include "audio.h"
#include <time.h>


void events_base(struct Prog *p, SDL_Event *evt)
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


void events_no_delay(struct Prog *p)
{
    float player_speed = 2.f;

    const Uint8 *keystates = SDL_GetKeyboardState(0);

    if (!p->game_over)
    {
        if (p->player->mode_data.mode == PLAYER_MODE_NORMAL)
        {
            if (keystates[SDL_SCANCODE_W])
                player_move(p->player, p->map, player_speed * cosf(p->player->angle), player_speed * -sinf(p->player->angle));

            if (keystates[SDL_SCANCODE_S])
                player_move(p->player, p->map, -player_speed * cosf(p->player->angle), -player_speed * -sinf(p->player->angle));

            if (keystates[SDL_SCANCODE_A])
                player_move(p->player, p->map, player_speed / 2.f * cosf(p->player->angle + M_PI / 2.f), player_speed / 2.f * -sinf(p->player->angle + M_PI / 2.f));

            if (keystates[SDL_SCANCODE_D])
                player_move(p->player, p->map, player_speed / 2.f * cosf(p->player->angle - M_PI / 2.f), player_speed / 2.f * -sinf(p->player->angle - M_PI / 2.f));
        }

        if (keystates[SDL_SCANCODE_RIGHT])
            p->player->angle -= 0.03f;

        if (keystates[SDL_SCANCODE_LEFT])
            p->player->angle += 0.03f;
    }
}


void events_keydown(struct Prog *p, SDL_Event *evt)
{
    if (!p->game_over)
    {
        switch (evt->key.keysym.sym)
        {
        case SDLK_SPACE:
        {
            if (p->game_over)
                break;

            struct Entity *hit = player_attack(p->player, p->entities, p->entities_size, p->map);

            if (hit && !hit->enemy_dead)
            {
                audio_play_sound("res/sfx/scream.wav");
                ++p->player->enemies_killed;
                entity_die(hit, p->rend, p->player->weapon);
            }
        } break;
        case SDLK_r:
        {
            if (p->player->reloading || p->player->weapon != WEAPON_GUN)
                break;

            audio_play_sound("res/sfx/reload.wav");
            p->player->reloading = true;
        } break;
        case SDLK_t:
        {
            int coll;
            SDL_Point endp = player_cast_ray(p->player, p->player->angle, p->map, p->entities, p->entities_size, &coll);

            SDL_Point grid_pos = {
                .x = (endp.x - (endp.x % p->map->tile_size)) / p->map->tile_size,
                .y = (endp.y - (endp.y % p->map->tile_size)) / p->map->tile_size
            };

            if (p->map->portal)
                portal_free(p->map->portal);

            p->map->portal = portal_alloc(grid_pos, coll);
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
    case SDLK_2:
        if (!p->game_over && p->player->weapon != WEAPON_GUN)
        {
            p->player->swinging = false;
            p->player->animation.knife_outstretched = false;
            p->player->animation.knife_pos.x = p->player->animation.knife_default_pos.x;
            p->player->animation.knife_pos.y = p->player->animation.knife_default_pos.y;

            p->player->animation.switching_weapon = WEAPON_GUN;
            audio_play_sound("res/sfx/gun_cocking.wav");
            p->player->weapon = WEAPON_GUN;
        }
        break;
    case SDLK_1:
        if (!p->game_over && !p->player->reloading && p->player->weapon != WEAPON_KNIFE)
        {
            p->player->animation.switching_weapon = WEAPON_KNIFE;
            audio_play_sound("res/sfx/knife_equip.wav");
            p->player->weapon = WEAPON_KNIFE;
        }
        break;
    }

#ifdef CHEATS_ON
    switch (evt->key.keysym.sym)
    {
    case SDLK_c:
        p->player->detect_collisions = !p->player->detect_collisions;

        if (p->player->detect_collisions)
            printf("Enabled collision detection\n");
        else
            printf("Disabled collision detection\n");
        break;
    case SDLK_h:
        p->player->ray_mode = RAY_HORIZONTAL;
        printf("Set wall rendering to only horizontal\n");
        break;
    case SDLK_v:
        p->player->ray_mode = RAY_VERTICAL;
        printf("Set wall rendering to only vertical\n");
        break;
    case SDLK_q:
        p->player->ray_mode = RAY_ALL;
        printf("Rendering walls normally\n");
        break;
    case SDLK_b:
        p->player->bullets_loaded += 100;
        printf("Added a few bullets\n");
        break;
    case SDLK_e:
        p->render_entities_over_walls = !p->render_entities_over_walls;

        if (p->render_entities_over_walls)
            printf("Rendering entities over walls\n");
        else
            printf("Rendering entities normally\n");
        break;
    case SDLK_p:
        ++p->player->health;
        printf("Added some health\n");
        break;
    case SDLK_x:
        p->player->ignore_walls_when_shooting = !p->player->ignore_walls_when_shooting;

        if (p->player->ignore_walls_when_shooting)
            printf("Bullets can now pass through walls\n");
        else
            printf("Bullets can no longer pass through walls\n");
        break;
    case SDLK_f:
        p->adjust_fisheye = !p->adjust_fisheye;

        if (p->adjust_fisheye)
            printf("Adjusting for fisheye effect\n");
        else
            printf("Not adjusting for fisheye effect\n");
        break;
    }
#endif
}


void events_mouse_down(struct Prog *p, SDL_Event *evt)
{
    SDL_SetRelativeMouseMode(SDL_TRUE);

    if (evt->button.button == SDL_BUTTON_LEFT)
        events_mouse_down_left(p, evt);
    else if (evt->button.button == SDL_BUTTON_RIGHT)
        events_mouse_down_right(p, evt);
}


void events_mouse_down_left(struct Prog *p, SDL_Event *evt)
{
    if (p->game_over)
        return;

    struct Entity *hit = player_attack(p->player, p->entities, p->entities_size, p->map);

    if (hit && !hit->enemy_dead)
    {
        audio_play_sound("res/sfx/scream.wav");
        ++p->player->enemies_killed;
        entity_die(hit, p->rend, p->player->weapon);
    }
}


void events_mouse_down_right(struct Prog *p, SDL_Event *evt)
{
    if (p->game_over || p->player->mode_data.mode != PLAYER_MODE_NORMAL || p->player->reloading)
        return;

    int collision_type;
    SDL_Point dst = player_cast_ray(p->player, p->player->angle, p->map, p->entities, p->entities_size, &collision_type);

    int xo = dst.x > p->player->pos.x ? -5 : 5;
    int yo = dst.y > p->player->pos.y ? -5 : 5;

    p->player->mode_data.mode = PLAYER_MODE_GRAPPLING;
    p->player->mode_data.grappling_dst = (SDL_Point){ .x = dst.x + xo, .y = dst.y + yo };
    p->player->mode_data.grappling_theta = atan2f(dst.y - p->player->pos.y, dst.x - p->player->pos.x);

    audio_play_sound("res/sfx/grapple.wav");
}

