#include <stdbool.h>
#include <SDL2/SDL.h>


typedef enum {
    LINE_STATE_IDLE,
    LINE_STATE_DIALING,
    LINE_STATE_OPERATOR_REQUEST,
    LINE_STATE_OPERATOR_REPLY,
    LINE_STATE_BUSY,
} sb_line_state_type;

typedef enum {
    CABLE_END_LEFT,
    CABLE_END_RIGHT
} sb_cable_end_type;


typedef struct sb_cable sb_cable_type;

typedef struct sb_customer {
    sb_line_state_type  line_state;
    SDL_Rect            port_rect;
    SDL_Rect            mugshot_rect;
    SDL_Color           color;
    sb_cable_type      *port_cable;
    sb_cable_end_type   port_cable_end;
} sb_customer_type;

struct sb_cable {
    sb_customer_type *left_customer;
    sb_customer_type *right_customer; 
    SDL_Rect          left_cable_base_rect;
    SDL_Rect          right_cable_base_rect;
    SDL_Rect          speak_button_rect;
    SDL_Rect          dial_button_rect;
    SDL_Color         color;
};


#define MAX_CUSTOMERS 32
#define MAX_CABLES 5

typedef struct game {
    size_t             customer_count;
    sb_customer_type   customers[MAX_CUSTOMERS];
    size_t             cable_count;
    sb_cable_type      cables[MAX_CABLES];
    sb_cable_type     *held_cable;
    sb_cable_end_type  held_cable_end;
} sb_game_type;


static inline bool
point_in_rect(int32_t x, int32_t y, SDL_Rect *rect)
{
    return (x > rect->x && x < rect->x + rect->w &&
            y > rect->y && y < rect->y + rect->h);
}


static inline void
rect_center(SDL_Rect *rect, int *x, int *y)
{
    *x = rect->x + rect->w / 2;
    *y = rect->y + rect->h / 2;
}



static void
update (uint32_t      frametime,
        sb_game_type *game)
{
}



static void
draw (SDL_Renderer *renderer,
      sb_game_type *game)
{
    ssize_t           i;
    int               startx;
    int               starty;
    int               endx;
    int               endy;
    sb_customer_type *cust;
    sb_cable_type    *cable;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &cust->port_rect);
        SDL_SetRenderDrawColor(renderer,
                               cust->color.r,
                               cust->color.g,
                               cust->color.b,
                               cust->color.a);
        SDL_RenderDrawRect(renderer, &cust->mugshot_rect);
    }

    for (i = 0; i < game->cable_count; i++) {
        cable = &game->cables[i];
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &cable->left_cable_base_rect);
        SDL_RenderDrawRect(renderer, &cable->right_cable_base_rect);
        SDL_RenderDrawRect(renderer, &cable->speak_button_rect);
        SDL_RenderDrawRect(renderer, &cable->dial_button_rect);
    }

    // Draw any cables that are plugged in.
    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];
        if (cust->port_cable != NULL) {
            cable = cust->port_cable;

            if (cust->port_cable_end == CABLE_END_LEFT) {
                rect_center(&cable->left_cable_base_rect, &startx, &starty);
            } else {
                rect_center(&cable->right_cable_base_rect, &startx, &starty);
            }
            rect_center(&cust->port_rect, &endx, &endy);

            SDL_SetRenderDrawColor(renderer,
                                   cable->color.r,
                                   cable->color.g,
                                   cable->color.b,
                                   cable->color.a);
            SDL_SetRenderDrawColor(renderer,
                                   cable->color.r,
                                   cable->color.g,
                                   cable->color.b,
                                   cable->color.a);
            SDL_RenderDrawLine(renderer, startx, starty, endx, endy);
        }
    }

    // If we're currently holding a cable end, draw the cable.
    if (game->held_cable != NULL) {
        cable = game->held_cable;

        (void)SDL_GetMouseState(&endx, &endy);
        if (game->held_cable_end == CABLE_END_LEFT) {
            rect_center(&cable->left_cable_base_rect, &startx, &starty);
        } else {
            rect_center(&cable->right_cable_base_rect, &startx, &starty);
        }

        SDL_SetRenderDrawColor(renderer,
                               cable->color.r,
                               cable->color.g,
                               cable->color.b,
                               cable->color.a);
        SDL_RenderDrawLine(renderer, startx, starty, endx, endy);
    }

    SDL_RenderPresent(renderer);
}


static void
setup_game (sb_game_type *game)
{
    size_t            i;
    sb_customer_type *cust;
    sb_cable_type    *cable;

    // TODO: Eventually layout etc will be done per-level etc.
    game->customer_count = 16;
    game->cable_count = 3;

    game->held_cable = NULL;

    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];

        cust->mugshot_rect.x = (i % 4) * 100;
        cust->mugshot_rect.y = (i / 4) * 50;
        cust->mugshot_rect.w = 32;
        cust->mugshot_rect.h = 32;

        cust->port_rect.x = cust->mugshot_rect.x + cust->mugshot_rect.w;
        cust->port_rect.y = cust->mugshot_rect.y;
        cust->port_rect.w = cust->mugshot_rect.w;
        cust->port_rect.h = cust->mugshot_rect.h;

        cust->color.r = i * 15;
        cust->color.g = 255 - cust->color.r;
        cust->color.b = (cust->color.r * cust->color.g) % 255;
        cust->color.a = 255;
    }

    for (i = 0; i < game->cable_count; i++) {
        cable = &game->cables[i];

        cable->left_cable_base_rect.x = i * 100;
        cable->left_cable_base_rect.y = 400;
        cable->left_cable_base_rect.w = 32;
        cable->left_cable_base_rect.h = 32;

        cable->right_cable_base_rect = cable->left_cable_base_rect;
        cable->right_cable_base_rect.x =
            cable->right_cable_base_rect.x + cable->right_cable_base_rect.w;

        cable->speak_button_rect = cable->left_cable_base_rect;
        cable->speak_button_rect.y += 48;
        
        cable->dial_button_rect = cable->speak_button_rect;
        cable->dial_button_rect.y += 48;

        cable->color.r = i * 75;
        cable->color.g = 255 - cust->color.r;
        cable->color.b = (cust->color.r * cust->color.g) % 255;
        cable->color.a = 255;
    }
}


static void
handle_mouse_button(SDL_MouseButtonEvent *e,
                    sb_game_type         *game)
{
    size_t            i;
    sb_cable_type    *cable;
    sb_customer_type *cust;

    if (e->button != SDL_BUTTON_LEFT) {
        return;
    }

    if (e->type == SDL_MOUSEBUTTONDOWN) {
        if (game->held_cable == NULL) {
            // Check whether this is picking up a new cable end from the base.
            for (i = 0; i < game->cable_count; i++) {
                cable = &game->cables[i];
                if (point_in_rect(e->x, e->y, &cable->left_cable_base_rect)) {
                    game->held_cable = cable;
                    game->held_cable_end = CABLE_END_LEFT;
                } else if (point_in_rect(
                                  e->x, e->y, &cable->right_cable_base_rect)) {
                    game->held_cable = cable;
                    game->held_cable_end = CABLE_END_RIGHT;
                }
            }

            // Check whether this is picking up a cable end from a customer.
            for (i = 0; i < game->customer_count; i++) {
                cust = &game->customers[i];
                if (cust->port_cable != NULL &&
                    point_in_rect(e->x, e->y, &cust->port_rect)) {
                    game->held_cable = cust->port_cable;
                    game->held_cable_end = cust->port_cable_end;
                    cust->port_cable = NULL;
                }
            }
            
            // Check whether this is hitting a button.
            // TODO
        }
    } else if (e->type == SDL_MOUSEBUTTONUP) {
        // Check whether we're putting a cable somewhere.
        if (game->held_cable != NULL) {
            for (i = 0; i < game->customer_count; i++) {
                cust = &game->customers[i];

                // TODO: Allow putting back in the same port.
                if (point_in_rect(e->x, e->y, &cust->port_rect) &&
                    cust->port_cable == NULL) {

                    cust->port_cable = game->held_cable;
                    cust->port_cable_end = game->held_cable_end;
                    if (game->held_cable_end == CABLE_END_LEFT) {
                        game->held_cable->left_customer = cust;
                    } else {
                        game->held_cable->right_customer = cust;
                    }
                }
            }

            game->held_cable = NULL;
        }
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
    sb_game_type  game = {0};

    (void)SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Switchboard",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              800, 600,
                              SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    setup_game(&game);

    while (run) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                run = false;
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                handle_mouse_button(&e.button, &game);
                break;

            default:
                break;
            }
        }

        ticks = SDL_GetTicks();
        frametime = ticks - last_ticks;
        last_ticks = ticks;

        update(frametime, &game);
        draw(renderer, &game);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
