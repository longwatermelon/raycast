#include "prog.h"
#include <time.h>
#include <SDL_image.h>
#include <SDL_ttf.h>


int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    srand(time(0));

    struct Prog* p = prog_init();
    prog_mainloop(p);

    prog_cleanup(p);

    SDL_Quit();
    IMG_Quit();
    TTF_Quit();

	return 0;
}
