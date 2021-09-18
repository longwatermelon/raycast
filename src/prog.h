#ifndef PROG_H
#define PROG_H

#include "player.h"
#include <stdbool.h>
#include <SDL.h>

struct Prog
{
    bool running;

    SDL_Window* window;
    SDL_Renderer* rend;

    struct Player* player;
};

struct Prog* prog_init();
void prog_cleanup(struct Prog* p);

void prog_mainloop(struct Prog* p);
void prog_handle_events(struct Prog* p, SDL_Event* evt);

#endif

