#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "gamestate.h"
#include "menu_main.h"
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

    // TODO: Error handling basically everywhere!

    (void)SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Switchboard",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              800, 600,
                              SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
    (void)SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    (void)TTF_Init();

    sb_game_setup(renderer);
    sb_menu_main_setup(renderer);
    sb_gamestate_push(sb_menu_main_get_gamestate());

    while (run) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                run = false;
            } else {
                sb_gamestate_event(&e);
            }
        }

        ticks = SDL_GetTicks();
        frametime = ticks - last_ticks;
        last_ticks = ticks;

        sb_gamestate_update(frametime);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        sb_gamestate_draw(renderer);
        SDL_RenderPresent(renderer);
    }

    sb_menu_main_cleanup();
    sb_game_cleanup();

    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
