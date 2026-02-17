#include "game.h"
#include <stdio.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static Player player;

//Initialize player properties
void player_init(void)
{
    player.x = 400;
    player.y = 300;
    player.speed = 0.3; //Pixels per frame
}

//Update player position based on user input
void player_update(const Uint8 *keystate)
{
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

//Initialize game window and renderer
bool game_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
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

    player_init();

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
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    player_update(keystate);
}

//Render game objects to the screen via renderer object
void game_render(void)
{
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderClear(renderer);

    player_render();

    SDL_RenderPresent(renderer);
}

//Close game cleanly, free resources
void game_cleanup(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
