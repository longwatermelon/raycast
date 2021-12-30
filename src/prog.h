#ifndef PROG_H
#define PROG_H

#include "player.h"
#include "map.h"
#include "entity.h"
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define MAX_ENTITIES 15

struct Prog
{
    bool running;

    SDL_Window *window;
    SDL_Renderer *rend;

    TTF_Font *font;

    struct Player *player;
    struct Map *map;

    struct Entity **entities;
    size_t entities_size;

    SDL_Texture *tile_texture;
    SDL_Point image_size;

    SDL_Texture *crosshair_texture;

    bool game_over;
    bool restart;
    bool win;

    int nuts_collected;

    bool render_entities_over_walls;
    bool adjust_fisheye;
};

struct Prog *prog_init(SDL_Window *window, SDL_Renderer *rend);
void prog_cleanup(struct Prog *self);

void prog_mainloop(struct Prog *self);
void prog_handle_entity_interaction(struct Prog *self);
void prog_render_all(struct Prog *self);

void prog_add_entity(struct Prog *self, struct Entity *entity);
void prog_remove_entity(struct Prog *self, struct Entity *entity);

void prog_spawn_entity(struct Prog *self, int type, const char *sprite_path);

int prog_entity_count(struct Prog *self, int type);

#endif

