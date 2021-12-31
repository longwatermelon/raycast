#include "player.h"
#include "common.h"
#include "prog.h"
#include "audio.h"
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>


struct Player *player_init(SDL_Point pos, float angle, SDL_Renderer *rend)
{
    struct Player *self = malloc(sizeof(struct Player));
    self->pos = (SDL_FPoint){ .x = pos.x, .y = pos.y };

    self->angle = angle;
    self->angle_change = 0.f;

    self->ray_mode = RAY_ALL;

    self->health = 3;
    clock_gettime(CLOCK_MONOTONIC, &self->last_hurt_time);

    self->shooting = false;
    clock_gettime(CLOCK_MONOTONIC, &self->last_shot_time);
    self->reloading = false;
    self->bullets = 16;
    self->bullets_loaded = 16;

    self->swinging = false;

    self->enemies_killed = 0;

    self->mode_data.mode = PLAYER_MODE_NORMAL;
    self->mode_data.grappling_dst = (SDL_Point){ .x = -1, .y = -1 };
    self->mode_data.grappling_theta = 0.f;

    self->weapon = WEAPON_GUN;

    if (!(self->shot_texture = IMG_LoadTexture(rend, "res/gfx/gun_shoot.png")) ||
        !(self->gun_texture = IMG_LoadTexture(rend, "res/gfx/gun.png")) ||
        !(self->knife_texture = IMG_LoadTexture(rend, "res/gfx/knife.png")))
    {
        fprintf(stderr, "Player textures not found\n");
        return 0;
    }

    self->animation.gun_default_pos = (SDL_Point){ .x = 500, .y = 500 };
    self->animation.gun_pos = (SDL_Rect){ .x = 500, .y = 500 };
    self->animation.gun_at_bottom = false;

    self->animation.knife_default_pos = (SDL_Point){ .x = 0, .y = 600 };
    self->animation.knife_pos = (SDL_Rect){ .x = 0, .y = 800 };
    self->animation.knife_outstretched = false;

    self->animation.switching_weapon = -1;

    self->detect_collisions = true;
    self->ignore_walls_when_shooting = false;

    return self;
}


void player_cleanup(struct Player *self)
{
    SDL_DestroyTexture(self->shot_texture);
    SDL_DestroyTexture(self->gun_texture);

    free(self);
}


void player_render_weapon(struct Player *self, SDL_Renderer *rend)
{
    // Gun
    SDL_Texture *tex = self->shooting ? self->shot_texture : self->gun_texture;
    SDL_QueryTexture(tex, 0, 0, &self->animation.gun_pos.w, &self->animation.gun_pos.h);
    SDL_RenderCopy(rend, tex, 0, &self->animation.gun_pos);

    // Knife
    SDL_QueryTexture(self->knife_texture, 0, 0, &self->animation.knife_pos.w, &self->animation.knife_pos.h);
    SDL_RenderCopy(rend, self->knife_texture, 0, &self->animation.knife_pos);
}


void player_advance_animations(struct Player *self)
{
    if (self->reloading)
    {
        if (!self->animation.gun_at_bottom)
            self->animation.gun_pos.y += 20;
        else
            self->animation.gun_pos.y -= 20;

        if (!self->animation.gun_at_bottom && self->animation.gun_pos.y >= 2000)
        {
            self->bullets += self->bullets_loaded;
            self->bullets -= 16;
            self->bullets_loaded = 16;

            if (self->bullets < 0)
            {
                self->bullets_loaded += self->bullets;
                self->bullets = 0;
            }

            self->animation.gun_at_bottom = true;
        }

        if (self->animation.gun_at_bottom && self->animation.gun_pos.y <= self->animation.gun_default_pos.y)
        {
            self->animation.gun_pos.y = self->animation.gun_default_pos.y;

            self->reloading = false;
            self->animation.gun_at_bottom = false;
        }
    }

    if (self->swinging)
    {
        if (!self->animation.knife_outstretched)
        {
            self->animation.knife_pos.x += 30;
            self->animation.knife_pos.y += 50;
        }
        else
        {
            self->animation.knife_pos.x -= 30;
            self->animation.knife_pos.y -= 50;
        }

        if (self->animation.knife_pos.x >= 170)
            self->animation.knife_outstretched = true;

        if (self->animation.knife_outstretched && self->animation.knife_pos.x <= self->animation.knife_default_pos.x)
        {
            self->animation.knife_pos.x = self->animation.knife_default_pos.x;
            self->animation.knife_pos.y = self->animation.knife_default_pos.y;

            self->swinging = false;
            self->animation.knife_outstretched = false;
        }
    }

    if (self->animation.switching_weapon != -1)
    {
        if (self->animation.switching_weapon == WEAPON_GUN) // Switch to gun
        {
            if (self->animation.knife_pos.y + 40 <= 800 && self->animation.gun_pos.y - 40 >= self->animation.gun_default_pos.y)
            {
                self->animation.knife_pos.y += 40;
                self->animation.gun_pos.y -= 40;
            }
            else
            {
                self->animation.switching_weapon = -1;
                self->animation.knife_pos.y = 800;
                self->animation.gun_pos.y = self->animation.gun_default_pos.y;
            }
        }
        else if (self->animation.switching_weapon == WEAPON_KNIFE) // Switch to knife
        {
            if (self->animation.gun_pos.y + 40 <= 800 && self->animation.knife_pos.y - 40 >= self->animation.knife_default_pos.y)
            {
                self->animation.gun_pos.y += 40;
                self->animation.knife_pos.y -= 40;
            }
            else
            {
                self->animation.switching_weapon = -1;
                self->animation.gun_pos.y = 800;
                self->animation.knife_pos.y = self->animation.knife_default_pos.y;
            }
        }
    }
}


void player_move(struct Player *self, struct Map *map, float x, float y)
{
    if (self->reloading)
    {
        x *= 0.5f;
        y *= 0.5f;
    }

    int xo = (x > 0 ? 5 : -5);
    int yo = (y > 0 ? 5 : -5);

    SDL_Point grid_pos = {
        (int)((self->pos.x + xo) - ((int)(self->pos.x + xo) % map->tile_size)) / map->tile_size,
        (int)((self->pos.y + yo) - ((int)(self->pos.y + yo) % map->tile_size)) / map->tile_size
    };

    SDL_Point new_grid_pos = {
        (int)((self->pos.x + xo + x) - ((int)(self->pos.x + xo + x) % map->tile_size)) / map->tile_size,
        (int)((self->pos.y + yo + y) - ((int)(self->pos.y + yo + y) % map->tile_size)) / map->tile_size
    };

    // Separate x and y collision checks so that player can still move in directions that aren't occupied by obstacles after colliding with something
    if (self->detect_collisions)
    {
        if (map->layout[grid_pos.y * map->size.x + new_grid_pos.x] == '.')
            self->pos.x += x;

        if (map->layout[new_grid_pos.y * map->size.x + grid_pos.x] == '.')
            self->pos.y += y;
    }
    else
    {
        self->pos.x += x;
        self->pos.y += y;
    }

    self->angle += self->angle_change;

    // Keep angle between 0 and 2pi
    self->angle = common_restrict_angle(self->angle);
}


void player_execute_mode(struct Player *self)
{
    switch (self->mode_data.mode)
    {
    case PLAYER_MODE_GRAPPLING:
    {
        self->pos.x += 7.f * cosf(self->mode_data.grappling_theta);
        self->pos.y += 7.f * sinf(self->mode_data.grappling_theta);

        if (fabsf(self->pos.x - self->mode_data.grappling_dst.x) < 10.f &&
            fabsf(self->pos.y - self->mode_data.grappling_dst.y) < 10.f)
        {
            self->mode_data.mode = PLAYER_MODE_NORMAL;
            self->pos.x = self->mode_data.grappling_dst.x;
            self->pos.y = self->mode_data.grappling_dst.y;

            self->mode_data.grappling_dst = (SDL_Point){ .x = -1, .y = -1 };
            self->mode_data.grappling_theta = 0.f;
        }
    } break;
    default:
        break;
    }
}


struct Entity *player_attack(struct Player *self, struct Entity **entities, size_t entities_size, struct Map *map)
{
    switch (self->weapon)
    {
    case WEAPON_GUN:
    {
        if (self->reloading)
            break;

        if (self->bullets_loaded <= 0)
        {
            audio_play_sound("res/sfx/gunshot_dry.wav");
            break;
        }

        self->shooting = true;
        clock_gettime(CLOCK_MONOTONIC, &self->last_shot_time);
        --self->bullets_loaded;

        audio_play_sound("res/sfx/gunshot.wav");

        return player_shoot(self, entities, entities_size, map);
    } break;
    case WEAPON_KNIFE:
    {
        if (self->swinging)
            break;

        struct Entity *ret = player_slash(self, entities, entities_size, map);
        self->swinging = true;

        if (ret)
            audio_play_sound("res/sfx/stab.wav");
        else
            audio_play_sound("res/sfx/slash.wav");

        return ret;
    } break;
    default:
        break;
    }

    return 0;
}


SDL_Point player_cast_ray(struct Player *self, float angle, struct Map *map, struct Entity **entities, size_t entities_size, int *collision_type)
{
    if (angle > 2.f * M_PI)
        angle -= 2.f * M_PI;

    if (angle < 0.f)
        angle += 2.f * M_PI;

    SDL_Point horizontal = player_cast_ray_horizontal(self, angle, map);
    SDL_Point vertical = player_cast_ray_vertical(self, angle, map);

    SDL_Point diff_h = { .x = horizontal.x - self->pos.x, .y = horizontal.y - self->pos.y };
    SDL_Point diff_v = { .x = vertical.x - self->pos.x, .y = vertical.y - self->pos.y };

    unsigned long dist_h = sqrtf(diff_h.x * diff_h.x + diff_h.y * diff_h.y);
    unsigned long dist_v = sqrtf(diff_v.x * diff_v.x + diff_v.y * diff_v.y);

    // Collision type representing which side the ray hits
    if (dist_h < dist_v) // Horizontal
        *collision_type = (angle > M_PI / 2.f && angle < 3.f * M_PI / 2.f ? DIR_RIGHT : DIR_LEFT);
    else // Vertical
        *collision_type = (angle < M_PI ? DIR_DOWN : DIR_UP);

    if (self->ray_mode == RAY_HORIZONTAL)
        return horizontal;
    if (self->ray_mode == RAY_VERTICAL)
        return vertical;

    return dist_h < dist_v ? horizontal : vertical;
}


SDL_Point player_cast_ray_horizontal(struct Player *self, float angle, struct Map *map)
{
    // Cast ray that only intersects horizontal lines
    SDL_FPoint closest_horizontal;
    closest_horizontal.y = (int)self->pos.y - ((int)self->pos.y % map->tile_size) + (angle > M_PI ? map->tile_size : 0);
    closest_horizontal.x = self->pos.x + ((closest_horizontal.y - self->pos.y) / -tanf(angle));

    if (angle <= 0.001f || 2 * M_PI - angle <= 0.001f) // Facing right, almost undefined
        return (SDL_Point){ 1e5, self->pos.y };

    if (fabsf((float)M_PI - angle) <= 0.001f) // Facing left, almost undefined
        return (SDL_Point){ -1e5, self->pos.y };

    float tan_a = tanf(angle);

    while (true)
    {
        SDL_Point grid_pos = {
            .x = (closest_horizontal.x - (fmod(closest_horizontal.x, map->tile_size))) / map->tile_size,
            .y = (closest_horizontal.y - (fmod(closest_horizontal.y, map->tile_size))) / map->tile_size
        };

        if (angle < M_PI)
            grid_pos.y -= 1;

        // Out of bounds, no point in continuing
        if (grid_pos.y < 0 || grid_pos.y >= map->size.y || grid_pos.x < 0 || grid_pos.x >= map->size.x)
            return (SDL_Point){ .x = (int)closest_horizontal.x, .y = (int)closest_horizontal.y };

        if (map->layout[grid_pos.y * map->size.x + grid_pos.x] != '.')
            return (SDL_Point){ .x = (int)closest_horizontal.x, .y = (int)closest_horizontal.y };

        float dy = (angle < M_PI ? -map->tile_size : map->tile_size);

        closest_horizontal.y += dy;
        closest_horizontal.x += dy / -tan_a; // tanf(angle)
    }
}


SDL_Point player_cast_ray_vertical(struct Player *self, float angle, struct Map *map)
{
    // Cast ray that only intersects vertical lines
    SDL_FPoint closest_vertical;
    closest_vertical.x = (int)self->pos.x - ((int)self->pos.x % map->tile_size) + (angle < M_PI / 2.f || angle > 3 * M_PI / 2.f ? map->tile_size : 0);
    closest_vertical.y = self->pos.y + ((closest_vertical.x - self->pos.x) * -tanf(angle));

    if (fabsf((float)(M_PI / 2.f) - angle) <= 0.001f)
        return (SDL_Point){ self->pos.x, -1e5 };

    if (fabsf((float)(3 * M_PI / 2.f) - angle) <= 0.001f)
        return (SDL_Point){ self->pos.x, 1e5 };

    // For optimization
    float tan_a = tanf(angle);

    while (true)
    {
        SDL_Point grid_pos = {
            .x = (closest_vertical.x - (fmod(closest_vertical.x, map->tile_size))) / map->tile_size,
            .y = (closest_vertical.y - (fmod(closest_vertical.y, map->tile_size))) / map->tile_size
        };

        if (angle > M_PI / 2.f && angle < 3 * M_PI / 2.f)
            grid_pos.x -= 1;

         // Out of bounds, no point in continuing
        if (grid_pos.y < 0 || grid_pos.y >= map->size.y || grid_pos.x < 0 || grid_pos.x >= map->size.x)
            return (SDL_Point){ .x = (int)closest_vertical.x, .y = (int)closest_vertical.y };

        if (map->layout[grid_pos.y * map->size.x + grid_pos.x] != '.')
            return (SDL_Point){ .x = (int)closest_vertical.x, .y = (int)closest_vertical.y };

        float dx = (angle < M_PI / 2.f || angle > 3 * M_PI / 2.f ? map->tile_size : -map->tile_size);

        closest_vertical.x += dx;
        closest_vertical.y += dx * -tan_a; // tanf(angle)
    }
}


int player_cast_ray_entity(struct Player *self, float angle, struct Entity **entities, size_t entities_size, struct Entity **ignored_entities, size_t ignored_entities_size, int target_type, float *intersection, struct Entity **entity_hit)
{
    float shortest = -1;

    // for optimization
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    for (int i = 0; i < entities_size; ++i)
    {
        if (target_type != -1 && entities[i]->type != target_type)
            continue;

        bool is_ignored = false;

        for (int j = 0; j < ignored_entities_size; ++j)
        {
            if (entities[i] == ignored_entities[j])
            {
                is_ignored = true;
                break;
            }
        }

        if (is_ignored)
            continue;

        SDL_FPoint diff = {
            .x = entities[i]->pos.x - self->pos.x,
            .y = entities[i]->pos.y - self->pos.y
        };

        SDL_FPoint ray_vector = {
            .x = cos_a, // cosf(angle)
            .y = -sin_a // sinf(angle)
        };

        float dot_product = diff.x * ray_vector.x + diff.y * ray_vector.y;
        float dist_a = sqrtf(diff.x * diff.x + diff.y * diff.y);

        float cos_theta = dot_product / dist_a;
        if (cos_theta > 1.f)
            cos_theta = 1.f;

        if (cos_theta < -1.f)
            cos_theta = -1.f;

        float theta = acosf(cos_theta);

        float cross = diff.x * ray_vector.y - diff.y * ray_vector.x;

        if (theta < M_PI / 2.f)
        {
            float h = dist_a * tanf(theta);

            if (h <= entities[i]->width / 2.f)
            {
                float len = sqrtf(dist_a * dist_a + h * h);

                if (len < shortest || shortest == -1)
                {
                    shortest = len;

                    if (cross < 0)
                        *intersection = (entities[i]->width / 2.f) - h;
                    else
                        *intersection = (entities[i]->width / 2.f) + h;

                    *entity_hit = entities[i];
                }
            }
        }
    }

    return shortest;
}


struct Entity *player_shoot(struct Player *self, struct Entity **entities, size_t entities_size, struct Map *map)
{
    if (self->bullets_loaded <= 0 || self->reloading)
        return 0;

    float intersection;
    struct Entity *entity = 0;
    int entity_dist = player_cast_ray_entity(self, self->angle, entities, entities_size, 0, 0, ENTITY_ENEMY, &intersection, &entity);

    if (self->ignore_walls_when_shooting)
    {
        if (entity_dist != -1)
            return entity;
    }
    else
    {
        int collision_type;
        SDL_Point wall_vector = player_cast_ray(self, self->angle, map, entities, entities_size, &collision_type);
        SDL_Point diff = { .x = wall_vector.x - self->pos.x, .y = wall_vector.y - self->pos.y };
        int wall_dist = sqrtf(diff.x * diff.x + diff.y * diff.y);

        if (entity_dist != -1 && entity_dist < wall_dist)
            return entity;
    }

    return 0;
}


struct Entity *player_slash(struct Player *self, struct Entity **entities, size_t entities_size, struct Map *map)
{
    for (int i = 0; i < entities_size; ++i)
    {
        if (entities[i]->type != ENTITY_ENEMY || entities[i]->enemy_dead)
            continue;

        SDL_FPoint diff = {
            .x = entities[i]->pos.x - self->pos.x,
            .y = entities[i]->pos.y - self->pos.y
        };

        float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);

        if (dist < 40.f)
            return entities[i];
    }

    return 0;
}

