#include "map.h"
#include "common.h"


struct Map *map_init(const char *path, SDL_Point size, int tile_size)
{
    struct Map *self = malloc(sizeof(struct Map));

    self->layout = common_read_file(path);

    if (!self->layout)
    {
        fprintf(stderr, "Failed to initialize map; couldn't read layout\n");
        return 0;
    }

    self->size = size;
    self->tile_size = tile_size;

#ifdef RANDOMIZE_MAP
    for (int i = 0; i < strlen(self->layout); ++i)
    {
        self->layout[i] = (rand() % 10 > 1 ? '.' : '#');

        // Set map boundaries to all solid
        if (i <= self->size.x || i >= self->size.x * self->size.y - self->size.x ||
            i % self->size.x == 0 || i % self->size.x == self->size.x - 1)
            self->layout[i] = '#';
    }
#endif

    return self;
}


void map_cleanup(struct Map *self)
{
    free(self->layout);
    free(self);
}


SDL_FPoint map_get_random_empty_spot(struct Map *self)
{
    SDL_FPoint pos = {
        .x = rand() % (self->size.x * self->tile_size),
        .y = rand() % (self->size.y * self->tile_size)
    };

    SDL_Point grid_pos = {
        .x = (pos.x - ((int)pos.x % self->tile_size)) / self->tile_size,
        .y = (pos.y - ((int)pos.y % self->tile_size)) / self->tile_size
    };

    while (self->layout[grid_pos.y * self->size.x + grid_pos.x] != '.')
    {
        pos.x = rand() % (self->size.x * self->tile_size);
        pos.y = rand() % (self->size.y * self->tile_size);

        grid_pos.x = (pos.x - ((int)pos.x % self->tile_size)) / self->tile_size;
        grid_pos.y = (pos.y - ((int)pos.y % self->tile_size)) / self->tile_size;
    }

    return pos;
}

