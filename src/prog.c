#include "prog.h"
#include "common.h"


struct Prog* prog_init()
{
    struct Prog* p = malloc(sizeof(struct Prog));
    p->running = true;

    p->window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, SDL_WINDOW_SHOWN);
    p->rend = SDL_CreateRenderer(p->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    p->player = player_init((SDL_Point){ 300, 320 }, 5.543169f);
    p->map = common_read_file("map");
    p->map_width = 16;
    p->tile_size = 50;

    return p;
}


void prog_cleanup(struct Prog* p)
{
    SDL_DestroyRenderer(p->rend);
    SDL_DestroyWindow(p->window);

    free(p->map);

    free(p);
}


void prog_mainloop(struct Prog* p)
{
    SDL_Event evt;

    while (p->running)
    {
        prog_handle_events(p, &evt);

        player_move(p->player, p->map, p->map_width, p->tile_size);

        SDL_RenderClear(p->rend);

        /* prog_render_map(p); */
        /* player_render(p->player, p->rend, p->map, p->map_width, p->tile_size); */

        /* SDL_Point center = { */
        /*     .x = p->player->rect.x + p->player->rect.w / 2, */
        /*     .y = p->player->rect.y + p->player->rect.h / 2 */
        /* }; */

        SDL_SetRenderDrawColor(p->rend, 255, 0, 0, 255);
        int x_pos = 0;

        for (float i = p->player->angle + M_PI / 6.f; i > p->player->angle - M_PI / 6.f; i -= 0.01f) // Cast 100 rays
        {
            bool is_horizontal;
            SDL_Point endp = player_cast_ray(p->player, i, p->map, p->map_width, p->tile_size, &is_horizontal);
            int ray_length = sqrtf((endp.x - p->player->rect.x) * (endp.x - p->player->rect.x) + (endp.y - p->player->rect.y) * (endp.y - p->player->rect.y));

            // Adjust for fisheye effect
            float angle = p->player->angle - i;

            if (angle < 0.f)
                angle += 2.f * M_PI;

            if (angle > 2.f * M_PI)
                angle -= 2.f * M_PI;

            float dist = ray_length * cosf(angle);
            float line_height = (p->tile_size * 800.f) / dist;

            if (line_height > 800.f)
                line_height = 800.f;

            float line_offset = 400.f - line_height / 2.f;

            SDL_Rect rect = { .x = x_pos * 8, .y = (int)line_offset, .w = 8, .h = (int)line_height };

            SDL_SetRenderDrawColor(p->rend, 255, (is_horizontal ? 255 : 0), 0, 255);
            SDL_RenderFillRect(p->rend, &rect);

            /* SDL_RenderDrawLine(p->rend, x_pos * 8, line_offset, x_pos * 8, line_offset + line_height); */

            ++x_pos;
            /* if (is_horizontal) */
            /*     SDL_SetRenderDrawColor(p->rend, 255, 0, 0, 255); */
            /* else */
            /*     SDL_SetRenderDrawColor(p->rend, 0, 255, 0, 255); */

            /* SDL_RenderDrawLine(p->rend, center.x, center.y, endp.x, endp.y); */
        }

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
                p->player->angle_change = -.01f;
                break;
            case SDLK_LEFT:
                p->player->angle_change = .01f;
                break;
            case SDLK_r:
                p->player->ray_mode = RAY_ALL;
                break;
            case SDLK_h:
                p->player->ray_mode = RAY_HORIZONTAL;
                break;
            case SDLK_v:
                p->player->ray_mode = RAY_VERTICAL;
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

    for (int i = 0; i < strlen(p->map); ++i)
    {
        if (p->map[i] == '#')
        {
            SDL_Rect rect = {
                .x = (i % p->map_width) * p->tile_size,
                .y = ((i - (i % p->map_width)) / p->map_width) * p->tile_size,
                .w = p->tile_size,
                .h = p->tile_size
            };

            SDL_RenderFillRect(p->rend, &rect);
        }
    }
}

