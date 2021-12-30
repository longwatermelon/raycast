#ifndef EVENTS_H
#define EVENTS_H

#include <SDL2/SDL.h>

struct Prog;

void events_base(struct Prog *p, SDL_Event *evt);
void events_no_delay(struct Prog *p);

void events_keydown(struct Prog *p, SDL_Event *evt);

void events_mouse_down(struct Prog *p, SDL_Event *evt);
void events_mouse_down_left(struct Prog *p, SDL_Event *evt);
void events_mouse_down_right(struct Prog *p, SDL_Event *evt);

#endif

