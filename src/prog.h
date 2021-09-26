#ifndef PROG_H
#define PROG_H

#include "player.h"
#include "map.h"
#include "entity.h"
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>

struct Prog
{
    bool running;

    SDL_Window* window;
    SDL_Renderer* rend;

    TTF_Font* font;

    struct Player* player;
    struct Map* map;

    struct Entity** entities;
    size_t entities_size;

    SDL_Texture* tile_texture;
    SDL_Point image_size;

    SDL_Texture* gun_texture;
    SDL_Texture* shot_texture;
};

struct Prog* prog_init();
void prog_cleanup(struct Prog* p);

void prog_mainloop(struct Prog* p);

void prog_render_map(struct Prog* p);
void prog_render_gun(struct Prog* p);

void prog_add_entity(struct Prog* p);
void prog_remove_entity(struct Prog* p, struct Entity* entity);

#endif

