#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
struct Prog;

void render_3d_all(struct Prog *p);

// Return nearest wall length
int render_3d_wall(struct Prog *p, float angle, int col);
void render_3d_entity(struct Prog *p, float angle, int col, int ray_length_wall);

#endif

