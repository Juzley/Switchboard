#include <stdbool.h>
#include <SDL2/SDL.h>


int
main (int argc, char *argv[])
{
    SDL_Window         *window;
    SDL_Renderer       *renderer;
    SDL_Event           e;
    bool                run = true;
    float               last_ticks = 0.0f;
    float               ticks;
    float               frametime;

    (void)SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Platform",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              800, 600,
                              SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    while (run) {
        if (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                run = false;
                break;

            default:
                break;
            }
        }

        ticks = (float)SDL_GetTicks() / 1000.0f;
        frametime = ticks - last_ticks;
        last_ticks = ticks;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
