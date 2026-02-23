#include "game.h"
#include <stdio.h>

//Technical properties for SDL window and renderer
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

//Game state variables
static Player player;
static Attack attack;
static Mouse mouse;

//textures
static SDL_Texture *player_texture = NULL;
static SDL_Texture *sword_texture = NULL;

//Track mouse position and button states
void mouse_update(void)
{
    Uint32 buttons = SDL_GetMouseState(&mouse.x, &mouse.y);
    mouse.lmb = buttons & SDL_BUTTON(SDL_BUTTON_LEFT) ? 1 : 0;
    mouse.rmb = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT) ? 1 : 0;
}

//Initialize player properties
void player_init(void)
{
    player.x = 400;
    player.y = 300;
    player.speed = 1.0f; //Pixels per frame
}

//Update player position based on user input
void player_update(const Uint8 *keystate)
{
    //MOVEMENT CODE (WASD)
    bool W = keystate[SDL_SCANCODE_W];
    bool S = keystate[SDL_SCANCODE_S];
    bool A = keystate[SDL_SCANCODE_A];
    bool D = keystate[SDL_SCANCODE_D];

    float diagonal_speed = player.speed / 1.4142f; //Adjust speed for diagonal movement

    //Straight movement
    if(W && !A && !D) { player.y -= player.speed; }
    if(S && !A && !D) { player.y += player.speed; }
    if(A && !W && !S) { player.x -= player.speed; }
    if(D && !W && !S) { player.x += player.speed; }

    //Diagonal movement
    if(W && A) { player.y -= diagonal_speed; player.x -= diagonal_speed; }
    if(W && D) { player.y -= diagonal_speed; player.x += diagonal_speed; }
    if(S && A) { player.y += diagonal_speed; player.x -= diagonal_speed; }
    if(S && D) { player.y += diagonal_speed; player.x += diagonal_speed; }
}

//Render player sprite (placeholder as a rectangle for now)
void player_render(void)
{
    SDL_Rect rect = { (int)player.x, (int)player.y, 32, 32 };
    SDL_SetRenderDrawColor(renderer, 200, 70, 70, 255);
    SDL_RenderFillRect(renderer, &rect);
}

//Run when attack button pressed
void attack_start(void)
{
    if(attack.active) return; //Already active, ignore
    if(attack.cooldown > 0) return; //Still cooling down, ignore

    attack.width = 48;
    attack.height = 48;
    attack.damage = 1;
    attack.timer = 100; //Frames until attack disappears
    attack.cooldown_max = 20; //Frames until next attack can be started
    attack.active = 1;
    attack.swing_progress = 0.0f; //Reset animation progress

    attack.cooldown = attack.timer + attack.cooldown_max; //Start cooldown immediately after attack starts

    //Position attack animation in front of player based on cursor location
    float dx = mouse.x - (player.x + 16); //Player center x
    float dy = mouse.y - (player.y + 16); //Player center y
    float length = sqrtf(dx*dx + dy*dy);

    if(length != 0) {
        dx /= length; //Normalize direction
        dy /= length;
    }

    attack.dir_x = dx;
    attack.dir_y = dy;

    attack.range = 60.0f; //Distance from player center to attack center
    attack.angle_cos = cosf(90.0f * 3.14159f / 180.0f); //90 degree cone

    attack.x = player.x - 8 + dx * attack.range;
    attack.y = player.y - 8 + dy * attack.range;
}

//Update attack state during lifetime, deactivate when timer runs out
void attack_update(void)
{
    if(attack.cooldown > 0) {
        attack.cooldown--;
    }

    if(!attack.active) return;

    attack.swing_progress += 0.05f; //swing speed
    /*if(attack.swing_progress > 1.0f) {
        attack.swing_progress = 1.0f; //Cap swing progress at 1.0
    }*/

    attack.timer--;
    if(attack.timer <= 0) {
        attack.active = 0; //Deactivate attack after timer runs out
    }
}

//Visually render attack if active
void attack_render(void)
{
    if(!attack.active) return;

    float cx = player.x + 16; //Player center x
    float cy = player.y + 0; //Player center y

    float swing_point; //actual position of the sword during animation.
    swing_point = 1.5 / (1 + pow(2.71828f, -2 * (attack.swing_progress - 0.5f))); //logistic function for smooth swing animation
    swing_point -= 0.5f; //Center around 0
    
    if(swing_point > 1.0f) swing_point = 1.0f; //Cap swing point at 1 (failsafe)

    float base_angle = atan2f(attack.dir_y, attack.dir_x) * 180.0f / 3.14159f; //Base angle in degrees
    float swing_offset = 50.0f - swing_point * 100.0f; //Swing from -50° to +50°
    float sprite_offset = 45.0f; //Offset to position sprite correctly (depends on sprite design)
    attack.angle = base_angle + swing_offset + sprite_offset;

    SDL_Rect dest;
    dest.w = 64;
    dest.h = 64;
    dest.x = (int)(cx);
    dest.y = (int)(cy - 41);

    SDL_Point pivot = { 0, 60 }; //Pivot point for rotation (handle of the sword)

    SDL_RenderCopyEx(renderer, sword_texture, NULL, &dest, attack.angle, &pivot, SDL_FLIP_NONE);
    
}

//Hitbox calculation
//determine if attack hits a point (e.g. enemy position) based on attack direction and range
int attack_hits_point(float px, float py) //px and py are enemy position
{
    if(!attack.active) return 0;

    float cx = player.x + 16; //Player center x
    float cy = player.y + 16; //Player center y

    float vx = px - cx;
    float vy = py - cy;

    float dist = sqrtf(vx*vx + vy*vy);
    if(dist > attack.range) return 0; //Outside max range

    if(dist == 0) return 1; //Enemy directly on top of player, obviously a hit

    //Now we know enemy is within range, check if within cone angle
    vx /= dist;
    vy /= dist;

    float dot = vx * attack.dir_x + vy * attack.dir_y;

    return dot >= attack.angle_cos; //Hit if within cone angle
}

//Initialize game window and renderer
bool game_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return false;
    }

    //initialize texture loading for PNG files
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    window = SDL_CreateWindow(
        "Roguelike",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        0
    );

    if (!window) {
        printf("Window Error: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        return false;
    }

    //init textures
    sword_texture = IMG_LoadTexture(renderer, "assets/nightsedge.png");
    if (!sword_texture) {
        printf("Failed to load sword: %s\n", IMG_GetError());
        return false;
    }

    //init player stats
    player_init();
    attack.active = 0; //No active attack at start

    return true;
}

//Run constantly to update game state, handle inputs
void game_handle_events(bool *running)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            *running = false;
    }
}

//Update game state based on player input
void game_update(void)
{
    mouse_update();

    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    player_update(keystate);
    
    if(mouse.lmb) {
        attack_start();
    }

    attack_update();
}

//Render game objects to the screen via renderer object
void game_render(void)
{
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderClear(renderer);

    player_render();
    attack_render();

    SDL_RenderPresent(renderer);
}

//Close game cleanly, free resources and destroy textures
void game_cleanup(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(sword_texture);
    IMG_Quit();
    SDL_Quit();
}
