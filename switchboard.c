#include <stdbool.h>
#include <SDL2/SDL.h>
#include "game.h"


int
main (int argc, char *argv[])
{
    SDL_Window          *window;
    SDL_Renderer        *renderer;
    SDL_Event            e;
    bool                 run = true;
    uint32_t             last_ticks = 0;
    uint32_t             ticks;
    uint32_t             frametime;
    sb_game_handle_type  game;

    (void)SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Switchboard",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              800, 600,
                              SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    game = sb_game_setup();

    while (run) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                run = false;
            } else {
                sb_game_event(&e, game);
            }
        }

        ticks = SDL_GetTicks();
        frametime = ticks - last_ticks;
        last_ticks = ticks;

        sb_game_update(frametime, game);
        sb_game_draw(renderer, game);
    }

    sb_game_cleanup(game);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
