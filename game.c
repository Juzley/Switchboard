#include <SDL2/SDL.h>
#include "game.h"
#include "util.h"

/*
 * Max numbers of game entities, for sizing arrays.
 */
#define MAX_CUSTOMERS 32
#define MAX_CABLES 32


/*
 * The range of times (in ms) between new calls.
 */
#define NEW_CALL_TIME_MIN 1000
#define NEW_CALL_TIME_MAX 10000


/*
 * The range of times (in ms) a customer will wait without a response from the
 * operator.
 */
#define HANGUP_TIME_MIN 5000
#define HANGUP_TIME_MAX 10000


/*
 * The range of times (in ms) a customer will take to answer the phone.
 */
#define ANSWER_TIME_MIN 500
#define ANSWER_TIME_MAX 5000


/*
 * The range of times (in ms) a connected call will last.
 */
#define CALL_TIME_MIN 5000
#define CALL_TIME_MAX 20000


/*
 * Enumeration representing the state of an individual
 * customer on the switchboard.
 */
typedef enum {
    LINE_STATE_IDLE,
    LINE_STATE_DIALING,
    LINE_STATE_OPERATOR_REQUEST,
    LINE_STATE_ANSWERING,
    LINE_STATE_OPERATOR_REPLY,
    LINE_STATE_BUSY,
} sb_line_state_type;


/*
 * There is a circular depenency between cables and customers,
 * so need a forward declaration here.
 */
typedef struct sb_cable sb_cable_type;


/*
 * Structure containing all information about a customer.
 */
typedef struct sb_game_customer {
    size_t                   index;
    sb_line_state_type       line_state;
    uint32_t                 update_time;
    uint16_t                 score;
    SDL_Rect                 port_rect;
    SDL_Rect                 mugshot_rect;
    SDL_Rect                 lamp_rect;
    SDL_Rect                 light_rect;
    SDL_Color                color;
    sb_cable_type           *port_cable;
    struct sb_game_customer *target_cust;
} sb_game_customer_type;


/*
 * Structure containing all information about a cable, including
 * the associated buttons.
 */
struct sb_cable {
    size_t                 index;
    sb_game_customer_type *customer;
    SDL_Rect               cable_base_rect;
    SDL_Rect               speak_button_rect;
    SDL_Rect               dial_button_rect;
    bool                   speak_active;
    SDL_Color              color;
};


/*
 * Structure containing game state.
 */
typedef struct sb_game {
    uint32_t                gametime;
    uint32_t                next_call_time;
    size_t                  customer_count;
    sb_game_customer_type   customers[MAX_CUSTOMERS];
    size_t                  cable_count;
    sb_cable_type           cables[MAX_CABLES];
    sb_cable_type          *held_cable;
    SDL_Texture            *console_texture;
    SDL_Texture            *port_texture;
    SDL_Texture            *lamp_texture;
    SDL_Texture            *flash_texture;
} sb_game_type;


/*
 * Find an idle customer, or return NULL if there are none.
 */
static sb_game_customer_type *
sb_game_find_idle_customer (sb_game_handle_type game)
{
    size_t                 i;
    size_t                 idle_count = 0;
    size_t                 index;
    size_t                 result_num;
    sb_game_customer_type *result = NULL;

    /*
     * Count the number of idle customers.
     */
    for (i = 0; i < game->customer_count; i++) {
        if (game->customers[i].line_state == LINE_STATE_IDLE) {
            idle_count++;
        }
    }

    if (idle_count >= 0) {
        /*
         * Pick a random idle customer, loop through the array skipping
         * non-idle customers.
         */
        result_num = random_range(1, idle_count);
        for (i = 0; result_num > 0 && i < game->customer_count; i++) {
            if (game->customers[i].line_state == LINE_STATE_IDLE) {
                result_num--;
            }
        }
        result = &game->customers[i];
    }

    return result;
}


/*
 * Select a random target customer for a call - note that this may be a
 * customer who is not idle.
 */
static sb_game_customer_type *
sb_game_find_target_customer (sb_game_customer_type *src_cust,
                              sb_game_handle_type    game)
{
    size_t                 result_index;
    sb_game_customer_type *result;

    result_index = random_at_most(game->customer_count - 2);
    if (result_index >= src_cust->index) {
        result_index++;
    }

    return &game->customers[result_index];
}


/*
 * Find the customer a given customer is currently connected to, or NULL if
 * there is no connected customer.
 */
static sb_game_customer_type *
sb_game_find_connected_customer (sb_game_customer_type *cust,
                                 sb_game_handle_type    game)
{
    sb_game_customer_type *result = NULL;
    sb_cable_type         *cable = cust->port_cable;
    sb_cable_type         *matching_cable;

    if (cable != NULL) {
        matching_cable = &game->cables[
                    cable->index % 2 ? cable->index + 1 : cable->index - 1];
        result = matching_cable->customer;
    }

    return (result);
}
                                 


/*
 * Handle a mouse button down or up event.
 */
static void
sb_game_mouse_button_event (SDL_MouseButtonEvent *e,
                            sb_game_handle_type   game)
{
    size_t                 i;
    sb_cable_type         *cable;
    sb_game_customer_type *cust;
    sb_game_customer_type *other_cust;

    if (e->button != SDL_BUTTON_LEFT) {
        return;
    }

    if (e->type == SDL_MOUSEBUTTONDOWN) {
        for (i = 0; i < game->cable_count; i++) {
            /*
             * Check whether this is picking up a new cable end from its base.
             */
            cable = &game->cables[i];
            if (cable->customer == NULL &&
                sb_point_in_rect(e->x, e->y, &cable->cable_base_rect)) {
                game->held_cable = cable;
            }

            /*
             * Check whether we've hit a talk button.
             */
            if (cable->customer != NULL &&
                cable->customer->line_state == LINE_STATE_DIALING &&
                sb_point_in_rect(e->x, e->y, &cable->speak_button_rect)) {
                cable->customer->line_state = LINE_STATE_OPERATOR_REQUEST;
            }

            /*
             * Check whether we've hit a dial button.
             */
            if (cable->customer != NULL &&
                cable->customer->line_state == LINE_STATE_IDLE &&
                sb_point_in_rect(e->x, e->y, &cable->dial_button_rect)) {
                cable->customer->line_state = LINE_STATE_ANSWERING;
                cable->customer->update_time =
                                random_range(game->gametime + ANSWER_TIME_MIN,
                                             game->gametime + ANSWER_TIME_MAX);
            }
        }

        // Check whether this is picking up a cable end from a customer.
        for (i = 0; i < game->customer_count; i++) {
            cust = &game->customers[i];
            if (cust->port_cable != NULL &&
                sb_point_in_rect(e->x, e->y, &cust->port_rect)) {
                game->held_cable = cust->port_cable;

                /*
                 * Check if we've interupted a call, and move all involved
                 * customers back to idle.
                 */
                if (cust->line_state == LINE_STATE_OPERATOR_REQUEST) {
                    cust->line_state = LINE_STATE_IDLE;
                    // TODO: Lose points.
                } else if(cust->line_state == LINE_STATE_OPERATOR_REPLY ||
                          cust->line_state == LINE_STATE_BUSY) {
                    // TODO: Lose points.
                    cust->line_state = LINE_STATE_IDLE;
                    other_cust = sb_game_find_connected_customer(cust, game);
                    if (other_cust != NULL) {
                        other_cust->line_state = LINE_STATE_IDLE;
                    }
                }

                cust->port_cable = NULL;
            }
        }
    } else if (e->type == SDL_MOUSEBUTTONUP) {
        // Check whether we're putting a cable somewhere.
        if (game->held_cable != NULL) {
            for (i = 0; i < game->customer_count; i++) {
                cust = &game->customers[i];

                // TODO: Allow putting back in the same port.
                if (sb_point_in_rect(e->x, e->y, &cust->port_rect) &&
                    cust->port_cable == NULL) {
                    cust->port_cable = game->held_cable;
                    game->held_cable->customer = cust;
                }
            }

            game->held_cable = NULL;
        }
    }
}


/*
 * See comment in game.h for more details.
 */
void
sb_game_event (SDL_Event           *e,
               sb_game_handle_type  game)
{
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        sb_game_mouse_button_event(&e->button, game);
        break;

    default:
        break;
    }
}


static void
sb_game_customer_update (sb_game_customer_type *cust,
                         sb_game_handle_type    game)
{
    switch (cust->line_state) {
    case LINE_STATE_DIALING:
        // TODO: Lose points for this.
        cust->line_state = LINE_STATE_IDLE;
        break;

    case LINE_STATE_ANSWERING:
        cust->line_state = LINE_STATE_OPERATOR_REPLY;
        break;

    case LINE_STATE_IDLE:
    default:
        break;
    }
}


/*
 * See comment in game.h for more details.
 */
void
sb_game_update (uint32_t            frametime,
                sb_game_handle_type game)
{
    sb_game_customer_type *src_cust;
    sb_game_customer_type *tgt_cust;
    sb_game_customer_type *cust;
    size_t                 i;

    game->gametime += frametime;

    if (game->gametime >= game->next_call_time) {
        /*
         * Initiate a new call.
         */
        src_cust = sb_game_find_idle_customer(game);
        tgt_cust = sb_game_find_target_customer(src_cust, game);

        src_cust->line_state = LINE_STATE_DIALING;
        src_cust->target_cust = tgt_cust;
        src_cust->update_time = random_range(game->gametime + HANGUP_TIME_MIN,
                                             game->gametime + HANGUP_TIME_MAX);

        game->next_call_time = random_range(
                                        game->gametime + NEW_CALL_TIME_MIN,
                                        game->gametime + NEW_CALL_TIME_MAX);
    }

    /*
     * Perform any required customer transitions.
     */
    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];
        if (cust->update_time != 0 && game->gametime >= cust->update_time) {
            sb_game_customer_update(cust, game);
        }
    }
}


/*
 * See comment in game.h for more details.
 */
void
sb_game_draw (SDL_Renderer        *renderer,
              sb_game_handle_type  game)
{
    ssize_t                i;
    int                    startx;
    int                    starty;
    int                    endx;
    int                    endy;
    sb_game_customer_type *cust;
    sb_cable_type         *cable;
    SDL_Rect               rect;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    rect.x = 0;
    rect.y = 0;
    rect.w = 800;
    rect.h = 400;
    SDL_RenderCopy(renderer, game->console_texture, NULL, &rect);

    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderCopy(renderer, game->port_texture, NULL, &cust->port_rect);
        SDL_RenderCopy(renderer, game->lamp_texture, NULL, &cust->lamp_rect);

        SDL_SetRenderDrawColor(renderer,
                               cust->color.r,
                               cust->color.g,
                               cust->color.b,
                               cust->color.a);

        if (cust->line_state == LINE_STATE_DIALING) {
            SDL_RenderFillRect(renderer, &cust->mugshot_rect);
        } else {
            SDL_RenderDrawRect(renderer, &cust->mugshot_rect);
        }
    }

    for (i = 0; i < game->cable_count; i++) {
        cable = &game->cables[i];
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &cable->cable_base_rect);
        SDL_RenderDrawRect(renderer, &cable->speak_button_rect);
        SDL_RenderDrawRect(renderer, &cable->dial_button_rect);
    }

    // Draw any cables that are plugged in.
    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];
        if (cust->port_cable != NULL) {
            cable = cust->port_cable;
            sb_rect_center(&cable->cable_base_rect, &startx, &starty);
            sb_rect_center(&cust->port_rect, &endx, &endy);

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
        sb_rect_center(&cable->cable_base_rect, &startx, &starty);

        SDL_SetRenderDrawColor(renderer,
                               cable->color.r,
                               cable->color.g,
                               cable->color.b,
                               cable->color.a);
        SDL_RenderDrawLine(renderer, startx, starty, endx, endy);
    }

    // Draw "conversations" for customers who are talking to the operator.
    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];
        if (cust->line_state == LINE_STATE_OPERATOR_REQUEST) {
            rect = cust->mugshot_rect;
            rect.x += 16;
            rect.y += 16;
            SDL_SetRenderDrawColor(renderer,
                                   cust->target_cust->color.r,
                                   cust->target_cust->color.g,
                                   cust->target_cust->color.b,
                                   cust->target_cust->color.a);
            SDL_RenderDrawRect(renderer, &rect);
        } else if (cust->line_state == LINE_STATE_OPERATOR_REPLY) {
            rect = cust->mugshot_rect;
            rect.x += 16;
            rect.y += 16;
            SDL_SetRenderDrawColor(renderer,
                                   cust->color.r,
                                   cust->color.g,
                                   cust->color.b,
                                   cust->color.a);
            SDL_RenderDrawRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}


/*
 * See comment in game.h for more details.
 */
sb_game_handle_type
sb_game_setup (SDL_Renderer *renderer)
{
    size_t                 i;
    size_t                 j;
    sb_game_customer_type *cust;
    sb_cable_type         *cable;
    sb_game_handle_type    game;
    uint8_t                columns;
    uint8_t                rows;
    uint32_t               column_spacing;
    uint32_t               row_spacing;

    game = calloc(1, sizeof(*game));
    
    // TODO: Proper media loading.
    game->console_texture = load_texture("media/console.png", renderer);
    game->port_texture = load_texture("media/port.png", renderer);
    game->lamp_texture = load_texture("media/lamp.png", renderer);
    game->flash_texture = load_texture("media/light.png", renderer);


    // TODO: Eventually layout etc will be done per-level etc.
    game->customer_count = 16;
    columns = 4;
    rows = game->customer_count / columns;

    game->cable_count = 6;

    game->held_cable = NULL;

    game->gametime = 0;
    game->next_call_time = random_range(NEW_CALL_TIME_MIN, NEW_CALL_TIME_MAX);

    column_spacing = 800 / (columns + 1);
    row_spacing = 400 / (rows + 1);
    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];

        cust->index = i;
        cust->line_state = LINE_STATE_IDLE;

        cust->mugshot_rect.x = column_spacing * ((i % columns) + 1) - 40;
        cust->mugshot_rect.y = row_spacing * ((i / columns) + 1) - 24;
        cust->mugshot_rect.w = 48;
        cust->mugshot_rect.h = 48;

        cust->port_rect.x = cust->mugshot_rect.x + cust->mugshot_rect.w;
        cust->port_rect.y = cust->mugshot_rect.y;
        cust->port_rect.w = 32;
        cust->port_rect.h = 32;
        
        cust->lamp_rect.x = cust->port_rect.x;
        cust->lamp_rect.y = cust->port_rect.y + cust->port_rect.h;
        cust->lamp_rect.w = 32;
        cust->lamp_rect.h = 16;

        cust->light_rect.x = cust->lamp_rect.x + 7;
        cust->light_rect.y = cust->lamp_rect.y - 1;
        cust->light_rect.w = 18;
        cust->light_rect.h = 18;

        cust->color.r = i * 15;
        cust->color.g = 255 - cust->color.r;
        cust->color.b = (cust->color.r * cust->color.g) % 255;
        cust->color.a = 255;
    }

    for (i = 0; i < game->cable_count / 2; i++) {
        for (j = 0; j < 2; j++) {
            cable = &game->cables[i * 2 + j];

            cable->index = i * 2 + j;

            cable->cable_base_rect.x = i * 100 + j * 32;
            cable->cable_base_rect.y = 400;
            cable->cable_base_rect.w = 32;
            cable->cable_base_rect.h = 32;

            cable->speak_button_rect = cable->cable_base_rect;
            cable->speak_button_rect.y += 48;

            cable->dial_button_rect = cable->speak_button_rect;
            cable->dial_button_rect.y += 48;

            cable->color.r = i * 75;
            cable->color.g = 255 - cust->color.r;
            cable->color.b = (cust->color.r * cust->color.g) % 255;
            cable->color.a = 255;
        }
    }

    return (game);
}


/*
 * See comment in game.h for more details.
 */
void
sb_game_cleanup(sb_game_handle_type game)
{
    free_texture(game->flash_texture);
    free_texture(game->lamp_texture);
    free_texture(game->port_texture);
    free_texture(game->console_texture);
    free(game);
}
