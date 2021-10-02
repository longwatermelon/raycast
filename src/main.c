#include "prog.h"
#include "audio.h"
#include <time.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>


int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_Init(MIX_INIT_MOD);

    srand(time(0));

    SDL_Window* window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, SDL_WINDOW_SHOWN);
    SDL_Renderer* rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    audio_init();
    audio_play_music("res/sfx/shreksophone.wav");

    while (true)
    {
        struct Prog* p = prog_init(window, rend);
        prog_mainloop(p);

        if (!p->restart)
        {
            prog_cleanup(p);
            break;
        }

        prog_cleanup(p);
    }

    audio_cleanup();

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);

    SDL_Quit();
    IMG_Quit();
    TTF_Quit();
    Mix_Quit();

	return 0;
}
