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

