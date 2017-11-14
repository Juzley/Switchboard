#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "gamestate.h"
#include "menu_main.h"
#include "menu_pause.h"
#include "endgame.h"
#include "game.h"


static bool sb_run = true;


void
sb_exit (void)
{
    sb_run = false;
}


int
main (int argc, char *argv[])
{
    SDL_Window          *window;
    SDL_Renderer        *renderer;
    SDL_Event            e;
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
    sb_endgame_setup(renderer);
    sb_menu_pause_setup(renderer);
    sb_menu_main_setup(renderer);
    sb_gamestate_push(sb_menu_main_get_gamestate());

    while (sb_run) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                sb_run = false;
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

    sb_menu_pause_cleanup();
    sb_menu_main_cleanup();
    sb_endgame_cleanup();
    sb_game_cleanup();

    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
