#ifndef PLAYER_H
#define PLAYER_H

#include <SDL.h>

struct Player
{
    SDL_FRect rect;
    float speed;

    float angle;
    float angle_change;
};

struct Player* player_init(SDL_Point pos, int angle);
void player_cleanup(struct Player* p);

void player_render(struct Player* p, SDL_Renderer* rend);

void player_move(struct Player* p);

#endif

