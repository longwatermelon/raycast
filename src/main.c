#include "prog.h"
#include "audio.h"
#include <stdio.h>
#include <time.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>


int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_Init(MIX_INIT_MOD);

    srand(time(0));

    SDL_Window *window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, SDL_WINDOW_SHOWN);
    SDL_Renderer *rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    FILE *fp = fopen("sfx_mute", "r");

    if (fp)
    {
        char muted[10];
        fscanf(fp, "%s", muted);
        bool sound, music;

        if (strcmp(muted, "sound") == 0)
            sound = true;

        if (strcmp(muted, "music") == 0)
            music = true;

        if (strcmp(muted, "all") == 0)
        {
            music = true;
            sound = true;
        }

        audio_mute(sound, music);
        fclose(fp);
    }

    audio_init();
    audio_play_music("res/sfx/shreksophone.wav");

    while (true)
    {
        struct Prog *p = prog_init(window, rend);

        if (!p)
        {
            fprintf(stderr, "Game failed to initialize, aborting\n");
            goto cleanup;
        }

        prog_mainloop(p);

        if (!p->restart)
        {
            prog_cleanup(p);
            break;
        }

        prog_cleanup(p);
    }

cleanup:
    audio_cleanup();

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);

    SDL_Quit();
    IMG_Quit();
    TTF_Quit();
    Mix_Quit();

    return 0;
}

