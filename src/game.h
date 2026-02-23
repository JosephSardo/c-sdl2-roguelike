#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

typedef struct{
    float x;
    float y;
    float speed;
} Player;

typedef struct{
    //technical properties
    float x;
    float y;
    int width;
    int height;
    int active;

    //gameplay properties
    int timer;
    int damage;

    //cooldown properties
    int cooldown;
    int cooldown_max;

    //hitbox properties
    float dir_x;
    float dir_y;
    float range;
    float angle_cos;

    //animation properties
    float angle;
    float swing_progress;
} Attack;

typedef struct{
    int x;
    int y;
    //Mouse button states (1 for pressed, 0 for released)
    int lmb; 
    int rmb;
} Mouse;

void mouse_update(void);

//Player functions
void player_init(void);
void player_update(const Uint8 *keystate);
void player_render(void);

//Attack functions
void attack_start(void);
void attack_update(void);
void attack_render(void);
int attack_hits_point(float px, float py);

//Game functions
bool game_init(void);
void game_handle_events(bool *running);
void game_update(void);
void game_render(void);
void game_cleanup(void);

#endif