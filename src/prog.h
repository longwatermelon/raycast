#ifndef PROG_H
#define PROG_H

#include "player.h"
#include "map.h"
#include <stdbool.h>
#include <SDL.h>

struct Prog
{
    bool running;

    SDL_Window* window;
    SDL_Renderer* rend;

    struct Player* player;
    struct Map* map;

    SDL_Texture* tile_texture;
    SDL_Point image_size;
};

struct Prog* prog_init();
void prog_cleanup(struct Prog* p);

void prog_mainloop(struct Prog* p);
void prog_handle_events(struct Prog* p, SDL_Event* evt);

void prog_render_map(struct Prog* p);

#endif

