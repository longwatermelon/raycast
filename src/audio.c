#include "audio.h"

Mix_Music* g_music = 0;
int g_current_channel = 0;

Mix_Chunk** g_sounds_playing = 0;
size_t g_sounds_playing_size = 0;


void audio_init()
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
        printf("Audio error: %s\n", SDL_GetError());
    }

    Mix_AllocateChannels(32);

    g_sounds_playing_size = 32;
    g_sounds_playing = malloc(32 * sizeof(Mix_Music*));
    memset(g_sounds_playing, 0, 32 * sizeof(Mix_Music*));
}


void audio_cleanup()
{
    if (g_music)
        Mix_FreeMusic(g_music);

    for (int i = 0; i < g_sounds_playing_size; ++i)
    {
        if (g_sounds_playing[i])
            Mix_FreeChunk(g_sounds_playing[i]);
    }

    free(g_sounds_playing);
}


void audio_play_music(const char* path)
{
    audio_stop_music();

    g_music = Mix_LoadMUS(path);

    if (!g_music)
    {
        printf("Audio error: %s\n", SDL_GetError());
    }
    else
    {
        Mix_PlayMusic(g_music, -1);
    }
}


void audio_stop_music()
{
    if (g_music)
    {
        Mix_FreeMusic(g_music);
        g_music = 0;
    }
}


void audio_play_sound(const char* path)
{
    if (Mix_Playing(g_current_channel))
    {
        ++g_current_channel;
        return;
    }

    Mix_Chunk* chunk = Mix_LoadWAV(path);

    if (!chunk)
    {
        printf("Audio error: %s\n", SDL_GetError());
    }
    else
    {
        if (Mix_PlayChannel(g_current_channel, chunk, 0) == -1)
        {
            printf("Audio error: %s\n", SDL_GetError());
        }
        else
        {
            g_sounds_playing[g_current_channel] = chunk;
            ++g_current_channel;

            if (g_current_channel >= 31)
                g_current_channel = 0;
        }
    }
}


void audio_stop_finished_sounds()
{
    for (int i = 0; i < g_sounds_playing_size; ++i)
    {
        if (!g_sounds_playing[i])
            continue;

        if (!Mix_Playing(i))
        {
            Mix_FreeChunk(g_sounds_playing[i]);
            g_sounds_playing[i] = 0;
        }
    }
}
