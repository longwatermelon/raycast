#ifndef EVENTS_H
#define EVENTS_H

#include <SDL.h>

struct Prog;

void events_base(struct Prog* p, SDL_Event* evt);
void events_keydown(struct Prog* p, SDL_Event* evt);
void events_keyup(struct Prog* p, SDL_Event* evt);

#endif
