#include <stdbool.h>
#include <SDL2/SDL.h>


typedef enum {
    LINE_STATE_IDLE,
    LINE_STATE_DIALING,
    LINE_STATE_OPERATOR_REQUEST,
    LINE_STATE_OPERATOR_REPLY,
    LINE_STATE_BUSY,
} sb_line_state_type;


typedef struct sb_customer {
    sb_line_state_type line_state;
    SDL_Rect           port_rect;
    SDL_Rect           mugshot_rect;
    SDL_Color          color;
} sb_customer_type;


typedef ssize_t sb_customer_index_type;
#define CUSTOMER_INDEX_NONE -1

typedef struct cable {
    sb_customer_index_type left_customer_index;
    sb_customer_index_type right_customer_index;
} sb_cable_type;

#define MAX_CUSTOMERS 32
#define MAX_CABLES 5

typedef struct game {
    size_t           customer_count;
    sb_customer_type customers[MAX_CUSTOMERS];
    size_t           cable_count;
    sb_customer_type cables[MAX_CABLES];
} sb_game_type;



static void
update (uint32_t   frametime,
        game_type *game)
{
}



static void
draw (SDL_Renderer *renderer,
      sb_game_type *game)
{
    size_t            i;
    sb_customer_type *cust;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, cust->port_rect);
        SDL_SetRenderDrawColor(renderer,
                               cust->color.r,
                               cust->color.g,
                               cust->color.b,
                               cust->color.a);
        SDL_RenderDrawRect(renderer, cust->mugshot_rect);
    }

    SDL_RenderPresent(renderer);
}


static void
setup_game (sb_game_type *game)
{
    size_t i;

    game->customer_count = 16;
    game->cable_count = 3;

    for (i = 0; i < game->customer_count; i++) {
    }
}


int
main (int argc, char *argv[])
{
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Event     e;
    bool          run = true;
    uint32_t      last_ticks = 0;
    uint32_t      ticks;
    uint32_t      frametime;

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

        ticks = SDL_GetTicks();
        frametime = ticks - last_ticks;
        last_ticks = ticks;

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
