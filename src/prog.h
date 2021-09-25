#ifndef PROG_H
#define PROG_H

#include "player.h"
#include "map.h"
#include "entity.h"
#include <stdbool.h>
#include <SDL.h>

struct Prog
{
    bool running;

    SDL_Window* window;
    SDL_Renderer* rend;

    struct Player* player;
    struct Map* map;

    struct Entity** entities;
    size_t entities_size;

    SDL_Texture* tile_texture;
    SDL_Point image_size;

    bool shooting;
    SDL_Texture* gun_texture;
    SDL_Texture* shot_texture;
    clock_t last_shot_time;
};

struct Prog* prog_init();
void prog_cleanup(struct Prog* p);

void prog_mainloop(struct Prog* p);
void prog_handle_events(struct Prog* p, SDL_Event* evt);

void prog_render_3d(struct Prog* p);
void prog_render_map(struct Prog* p);
void prog_render_gun(struct Prog* p);

void prog_add_entity(struct Prog* p);
void prog_remove_entity(struct Prog* p, struct Entity* entity);

#endif

