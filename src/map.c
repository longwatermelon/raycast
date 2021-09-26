#include "map.h"
#include "common.h"


struct Map* map_init(const char* path, SDL_Point size, int tile_size)
{
    struct Map* m = malloc(sizeof(struct Map));

    m->layout = common_read_file(path);
    m->size = size;
    m->tile_size = tile_size;

    return m;
}


void map_cleanup(struct Map* m)
{
    free(m->layout);
    free(m);
}


SDL_FPoint map_get_random_empty_spot(struct Map* m)
{
    SDL_FPoint pos = {
        .x = rand() % (m->size.x * m->tile_size),
        .y = rand() % (m->size.y * m->tile_size)
    };

    SDL_Point grid_pos = {
        .x = (pos.x - ((int)pos.x % m->tile_size)) / m->tile_size,
        .y = (pos.y - ((int)pos.y % m->tile_size)) / m->tile_size
    };

    while (m->layout[grid_pos.y * m->size.x + grid_pos.x] == '#')
    {
        pos.x = rand() % (m->size.x * m->tile_size);
        pos.y = rand() % (m->size.y * m->tile_size);

        grid_pos.x = (pos.x - ((int)pos.x % m->tile_size)) / m->tile_size;
        grid_pos.y = (pos.y - ((int)pos.y % m->tile_size)) / m->tile_size;
    }

    return pos;
}

