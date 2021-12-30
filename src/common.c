#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


char *common_read_file(const char *path)
{
    char *contents = malloc(sizeof(char));
    contents[0] = '\0';

    FILE *fp = fopen(path, "r");
    char *line = 0;
    size_t len = 0;
    ssize_t read;

    if (!fp)
    {
        fprintf(stderr, "Couldn't read file %s\n", path);
        return 0;
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        unsigned long prev_length = strlen(contents);
        contents = realloc(contents, (strlen(contents) + read) * sizeof(char));

        memcpy(&contents[prev_length], line, (read - 1) * sizeof(char));
        contents[prev_length + read - 1] = '\0';
    }

    free(line);
    fclose(fp);

    return contents;
}


float common_restrict_angle(float angle)
{
    if (angle > 2.f * M_PI)
        angle -= 2.f * M_PI;

    if (angle < 0.f)
        angle += 2.f * M_PI;

    return angle;
}


SDL_Texture *common_render_text(SDL_Renderer *rend, TTF_Font *font, const char *text)
{
    if (strlen(text) == 0)
        return 0;

    SDL_Surface *surf = TTF_RenderText_Blended(font, text, (SDL_Color){ 255, 255, 255 });
    SDL_Texture *tex = SDL_CreateTextureFromSurface(rend, surf);

    SDL_FreeSurface(surf);
    return tex;
}


void common_display_statistic(SDL_Renderer *rend, TTF_Font *font, const char *text, int value, SDL_Point pos)
{
    int value_len = snprintf(0, 0, "%d", value);
    int length = strlen(text) + value_len + 1;
    char *display = malloc(sizeof(char) * length);
    snprintf(display, length, "%s%d", text, value);
    SDL_Texture *tex = common_render_text(rend, font, display);
    free(display);

    SDL_Rect tmp = { .x = pos.x, .y = pos.y };
    SDL_QueryTexture(tex, 0, 0, &tmp.w, &tmp.h);
    SDL_RenderCopy(rend, tex, 0, &tmp);

    SDL_DestroyTexture(tex);
}


float common_time_diff(struct timespec t1, struct timespec t2)
{
    return (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9;
}

