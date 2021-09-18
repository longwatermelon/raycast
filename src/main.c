#include "prog.h"


int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    struct Prog* p = prog_init();
    prog_mainloop(p);

    prog_cleanup(p);

    SDL_Quit();

	return 0;
}
