#include <SDL2/SDL.h>
#include "gamestate.h"
#include "game.h"
#include "util.h"
#include "math.h"

/*
 * Max numbers of game entities, for sizing arrays.
 */
#define MAX_CUSTOMERS 32
#define MAX_CABLES 32


/*
 * Range of times (in ms) between new calls.
 */
#define NEW_CALL_TIME_MIN 1000
#define NEW_CALL_TIME_MAX 10000


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
    LINE_STATE_COUNT
} sb_line_state_type;


/*
 * Structure representing the minumum and maximum times a customer will stay
 * in a given state.
 */
typedef struct {
    uint32_t min;
    uint32_t max;
} sb_line_state_update_range_type;


/*
 * An array of the min amd max times a customer stay in each state, indexed
 * by the state enum value.
 */
static sb_line_state_update_range_type
sb_game_line_state_update_ranges[LINE_STATE_COUNT] =
{
    { 0,     0 },     // LINE_STATE_IDLE
    { 5000,  10000 }, // LINE_STATE_DIALING
    { 20000, 30000 }, // LINE_STATE_OPERATOR_REQUEST
    { 500,   5000 },  // LINE_STATE_ANSWERING
    { 5000,  10000 }, // LINE_STATE_OPERATOR_REPLY
    { 8000,  40000 }, // LINE_STATE_BUSY
};


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
    uint32_t                 last_update;
    uint32_t                 next_update;
    SDL_Rect                 port_rect;
    SDL_Rect                 mugshot_rect;
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
    sb_cable_type          *active_cable;
    SDL_Texture            *console_texture;
    SDL_Texture            *panel_texture;
    SDL_Texture            *port_texture;
    SDL_Texture            *flash_texture;
    SDL_Texture            *plug_connected_texture;
    SDL_Texture            *plug_loose_texture;
    SDL_Texture            *mug_background_texture;
    SDL_Texture            *speech_bubble_texture;
} sb_game_type;


static sb_game_type sb_game;


static void
sb_game_update_customer_state (sb_game_customer_type *cust,
                               sb_game_type          *game,
                               sb_line_state_type     state)
{
    cust->line_state = state;
    cust->last_update = game->gametime;
    cust->next_update = random_range(
                 game->gametime + sb_game_line_state_update_ranges[state].min,
                 game->gametime + sb_game_line_state_update_ranges[state].max);
}


/*
 * Find an idle customer, or return NULL if there are none.
 */
static sb_game_customer_type *
sb_game_find_idle_customer (sb_game_type *game)
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

    if (idle_count > 0) {
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
                              sb_game_type          *game)
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
                                 sb_game_type          *game)
{
    sb_game_customer_type *result = NULL;
    sb_cable_type         *cable = cust->port_cable;
    sb_cable_type         *matching_cable;

    if (cable != NULL) {
        matching_cable = &game->cables[
                    cable->index % 2 == 0 ? cable->index + 1 : cable->index - 1];
        result = matching_cable->customer;
    }

    return (result);
}


static void
sb_game_talk_button_press (sb_cable_type *cable,
                           sb_game_type  *game)
{
    sb_game_customer_type *other_cust;

    if (cable->customer == NULL) {
        return;
    }

    if (game->active_cable == cable) {
        /*
         * Deactivate the cable.
         */
        game->active_cable = NULL;

        if (cable->customer->line_state == LINE_STATE_OPERATOR_REPLY) {
            other_cust = sb_game_find_connected_customer(
                                                    cable->customer, game);
            if (other_cust != NULL &&
                other_cust->target_cust == cable->customer) {
                /*
                 * TODO: Add Points
                 */
                sb_game_update_customer_state(cable->customer, game,
                                              LINE_STATE_BUSY);
                sb_game_update_customer_state(other_cust, game,
                                              LINE_STATE_BUSY);
                /*
                 * The above functions will have given the two customers a
                 * different update time - we want both customers to move back
                 * to idle at the same time.
                 */
                other_cust->next_update = cable->customer->next_update;
            }
        }
    } else {
        /*
         * Activate the cable.
         */
        game->active_cable = cable;

        if (cable->customer->line_state == LINE_STATE_DIALING) {
            sb_game_update_customer_state(cable->customer, game,
                                          LINE_STATE_OPERATOR_REQUEST);
        }
    }
}
                                 


/*
 * Handle a mouse button down or up event.
 */
static void
sb_game_mouse_button_event (SDL_MouseButtonEvent *e,
                            sb_game_type         *game)
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
            if (sb_point_in_rect(e->x, e->y, &cable->speak_button_rect)) {
                sb_game_talk_button_press(cable, game);
            }

            /*
             * Check whether we've hit a dial button.
             */
            if (cable->customer != NULL &&
                cable->customer->line_state == LINE_STATE_IDLE &&
                sb_point_in_rect(e->x, e->y, &cable->dial_button_rect)) {
                sb_game_update_customer_state(cable->customer, game,
                                              LINE_STATE_ANSWERING);
                game->active_cable = NULL;
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
                    sb_game_update_customer_state(cust, game, LINE_STATE_IDLE);
                    // TODO: Lose points.
                } else if(cust->line_state == LINE_STATE_OPERATOR_REPLY ||
                          cust->line_state == LINE_STATE_BUSY) {
                    // TODO: Lose points.
                    sb_game_update_customer_state(cust, game, LINE_STATE_IDLE);
                    other_cust = sb_game_find_connected_customer(cust, game);
                    if (other_cust != NULL) {
                        sb_game_update_customer_state(other_cust, game,
                                                      LINE_STATE_IDLE);
                    }
                }

                /*
                 * Unplugging the cable hangs up if it was active.
                 */
                if (game->held_cable == game->active_cable) {
                    game->active_cable = NULL;
                }

                cust->port_cable = NULL;
                game->held_cable->customer = NULL;
            }
        }
    } else if (e->type == SDL_MOUSEBUTTONUP) {
        // Check whether we're putting a cable somewhere.
        if (game->held_cable != NULL) {
            for (i = 0; i < game->customer_count; i++) {
                cust = &game->customers[i];

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
static void
sb_game_event (SDL_Event *e,
               void      *context)
{
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        sb_game_mouse_button_event(&e->button, &sb_game);
        break;

    default:
        break;
    }
}


static void
sb_game_customer_update (sb_game_customer_type *cust,
                         sb_game_type          *game)
{
    switch (cust->line_state) {
    case LINE_STATE_DIALING:
    case LINE_STATE_OPERATOR_REQUEST:
    case LINE_STATE_OPERATOR_REPLY:
        // TODO: Lose points for this.
        sb_game_update_customer_state(cust, game, LINE_STATE_IDLE);
        break;

    case LINE_STATE_ANSWERING:
        game->active_cable = cust->port_cable;
        sb_game_update_customer_state(cust, game, LINE_STATE_OPERATOR_REPLY);
        break;

    case LINE_STATE_BUSY:
        sb_game_update_customer_state(cust, game, LINE_STATE_IDLE);

    case LINE_STATE_IDLE:
    default:
        break;
    }
}


/*
 * See comment in game.h for more details.
 */
static void
sb_game_update (uint32_t  frametime,
                void     *context)
{
    sb_game_customer_type *src_cust;
    sb_game_customer_type *tgt_cust;
    sb_game_customer_type *cust;
    sb_game_type          *game = &sb_game;
    size_t                 i;

    game->gametime += frametime;

    if (game->gametime >= game->next_call_time) {
        /*
         * Initiate a new call.
         */
        src_cust = sb_game_find_idle_customer(game);
        tgt_cust = sb_game_find_target_customer(src_cust, game);

        sb_game_update_customer_state(src_cust, game, LINE_STATE_DIALING);
        src_cust->target_cust = tgt_cust;
        game->next_call_time = random_range(
                                        game->gametime + NEW_CALL_TIME_MIN,
                                        game->gametime + NEW_CALL_TIME_MAX);
    }

    /*
     * Perform any required customer transitions.
     */
    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];
        if (cust->next_update != 0 && game->gametime >= cust->next_update) {
            sb_game_customer_update(cust, game);
        }
    }
}


static void
sb_game_draw_cable_cord (SDL_Renderer *renderer,
                         int           startx,
                         int           starty,
                         int           endx,
                         int           endy,
                         SDL_Color     color,
                         sb_game_type *game)
{
    int      distx = startx - endx;
    int      disty = starty - endy;
    int      centerx;
    int      centery;
    float    line_length;
    int      render_count; 
    int      i;
    SDL_Rect rect;

    line_length = sqrtf(distx * distx + disty * disty);
    render_count = line_length / 2;

    /*
     * If the line is too short, return.
     */
    if (render_count == 0) {
        return;
    }

    SDL_SetTextureColorMod(game->plug_connected_texture,
                           color.r,
                           color.g,
                           color.b);

    for (i = 0; i < render_count; i++) {
        centerx = startx + ((((float)i / (float)render_count - 1)) * distx);
        centery = starty + ((((float)i / (float)render_count - 1)) * disty);

        rect.x = centerx - 4;
        rect.y = centery - 4;
        rect.w = 8;
        rect.h = 8;
        //SDL_RenderFillRect(renderer, &rect);
        SDL_RenderCopy(renderer, game->plug_connected_texture, NULL, &rect);
        
    }
}


/*
 * See comment in game.h for more details.
 */
static void
sb_game_draw (SDL_Renderer *renderer,
              void         *context)
{
    ssize_t                i;
    int                    startx;
    int                    starty;
    int                    endx;
    int                    endy;
    float                  progress;
    sb_game_customer_type *cust;
    sb_cable_type         *cable;
    SDL_Rect               rect;
    sb_game_type          *game = &sb_game;

    /*
     * Draw the background
     */
    rect.x = 110;
    rect.y = 10;
    rect.w = 580;
    rect.h = 480;
    SDL_RenderCopy(renderer, game->panel_texture, NULL, &rect);
    rect.x = 0;
    rect.y = 500;
    rect.w = 800;
    rect.h = 100;
    SDL_RenderCopy(renderer, game->console_texture, NULL, &rect);

    /*
     * Draw customer ports + mugshots.
     */
    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];

        rect = cust->mugshot_rect;
        rect.x += 4;
        rect.y += 4;
        rect.w -= 8;
        rect.h -= 8;
        SDL_SetRenderDrawColor(renderer,
                               cust->color.r,
                               cust->color.g,
                               cust->color.b,
                               cust->color.a);
        SDL_RenderFillRect(renderer, &rect);

        if (cust->line_state != LINE_STATE_IDLE &&
            cust->line_state != LINE_STATE_ANSWERING) {
            progress = ((float)(cust->next_update - game->gametime) /
                        (float)(cust->next_update - cust->last_update));
            rect.y += rect.h - rect.h * progress + 1;
            rect.h *= progress;

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
            SDL_RenderFillRect(renderer, &rect);
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        SDL_RenderCopy(renderer, game->port_texture, NULL, &cust->port_rect);
        SDL_RenderCopy(renderer, game->mug_background_texture, NULL,
                       &cust->mugshot_rect);


        if (cust->line_state == LINE_STATE_DIALING ||
            cust->line_state == LINE_STATE_ANSWERING) {
            if ((cust->next_update - game->gametime) % 1000 > 500) {
                SDL_RenderCopy(renderer, game->flash_texture, NULL,
                               &cust->light_rect);
            }
        } else if (cust->line_state == LINE_STATE_BUSY ||
                   cust->line_state == LINE_STATE_OPERATOR_REQUEST ||
                   cust->line_state == LINE_STATE_OPERATOR_REPLY) {
            SDL_RenderCopy(renderer, game->flash_texture, NULL,
                           &cust->light_rect);
        }
    }

    /*
     * Draw the cables bases, buttons etc.
     */
    for (i = 0; i < game->cable_count; i++) {
        cable = &game->cables[i];
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        /*
         * The cable is not held or plugged in - draw the connector at the
         * base.
         */
        if (cable->customer == NULL && game->held_cable != cable) {
            SDL_RenderCopy(renderer, game->plug_loose_texture, NULL,
                           &cable->cable_base_rect);
        }
        
        if (game->active_cable == cable) {
            SDL_RenderFillRect(renderer, &cable->speak_button_rect);
        } else{
            SDL_RenderDrawRect(renderer, &cable->speak_button_rect);
        }

        if (cable->customer &&
            cable->customer->line_state == LINE_STATE_ANSWERING) {
            SDL_RenderFillRect(renderer, &cable->dial_button_rect);
        } else {
            SDL_RenderDrawRect(renderer, &cable->dial_button_rect);
        }
    }

    /*
     * Draw any cables that are plugged in.
     */
    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];
        if (cust->port_cable != NULL) {
            cable = cust->port_cable;
            rect.x = cust->port_rect.x + 4;
            rect.y = cust->port_rect.y + 4;
            rect.w = 24;
            rect.h = 24;
            SDL_RenderCopy(renderer, game->plug_connected_texture, NULL,
                           &rect);

            sb_rect_center(&cable->cable_base_rect, &startx, &starty);
            sb_rect_center(&cust->port_rect, &endx, &endy);
            SDL_SetRenderDrawColor(renderer,
                                   cable->color.r,
                                   cable->color.g,
                                   cable->color.b,
                                   cable->color.a);
            SDL_RenderDrawLine(renderer, startx, starty, endx, endy);
            sb_game_draw_cable_cord(renderer, endx, endy, startx, starty,
                                    cable->color, game);
        }
    }

    /*
     * If we're currently holding a cable end, draw the cable.
     */
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
        sb_game_draw_cable_cord(renderer, endx, endy, startx, starty,
                                cable->color, game);

        rect = game->held_cable->cable_base_rect;
        rect.x = endx - rect.w / 2;
        rect.y = endy - rect.h / 2;
        SDL_RenderCopy(renderer, game->plug_loose_texture, NULL, &rect);
    }

    // Draw "conversations" for customers who are talking to the operator.
    for (i = 0; i < game->customer_count; i++) {
        cust = &game->customers[i];
        if ((cust->line_state == LINE_STATE_OPERATOR_REQUEST ||
             cust->line_state == LINE_STATE_OPERATOR_REPLY) &&
             cust->port_cable == game->active_cable) {
            rect = cust->mugshot_rect;
            rect.x += 24;
            rect.y -= 48;
            rect.w = 100;
            rect.h = 76;
            SDL_RenderCopy(renderer, game->speech_bubble_texture, NULL,
                           &rect);

            if (cust->line_state == LINE_STATE_OPERATOR_REQUEST) {
                rect.x += 30;
                rect.y += 12;
                rect.w = 40;
                rect.h = 40;
                SDL_SetRenderDrawColor(renderer,
                                       cust->target_cust->color.r,
                                       cust->target_cust->color.g,
                                       cust->target_cust->color.b,
                                       cust->target_cust->color.a);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}


/*
 * See comment in game.h for more details.
 */
void
sb_game_setup (SDL_Renderer *renderer)
{
    size_t                 i;
    size_t                 j;
    sb_game_customer_type *cust;
    sb_cable_type         *cable;
    sb_game_type          *game;
    uint8_t                columns;
    uint8_t                rows;
    uint32_t               column_spacing;
    uint32_t               row_spacing;

    game = &sb_game;
    
    // TODO: Proper media loading.
    game->panel_texture = load_texture("media/panel.png", renderer);
    game->console_texture = load_texture("media/console.png", renderer);
    game->port_texture = load_texture("media/port.png", renderer);
    game->flash_texture = load_texture("media/light.png", renderer);
    game->plug_connected_texture = load_texture("media/plug_connected.png",
                                                renderer);
    game->plug_loose_texture = load_texture("media/plug_loose.png", renderer);
    game->mug_background_texture = load_texture("media/mug_background.png",
                                                renderer);
    game->speech_bubble_texture = load_texture("media/speech_bubble.png",
                                               renderer);


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
        cust->mugshot_rect.w = 64;
        cust->mugshot_rect.h = 64;

        cust->port_rect.x = cust->mugshot_rect.x + cust->mugshot_rect.w;
        cust->port_rect.y = cust->mugshot_rect.y;
        cust->port_rect.w = 32;
        cust->port_rect.h = 64;
        
        cust->light_rect.w = 16;
        cust->light_rect.h = 16;
        cust->light_rect.x = cust->port_rect.x + 8;
        cust->light_rect.y = cust->port_rect.y + 40; 

        cust->color.r = i * 15;
        cust->color.g = 255 - cust->color.r;
        cust->color.b = (cust->color.r * cust->color.g) % 255;
        cust->color.a = 255;
    }

    column_spacing = 800 / (game->cable_count / 2 + 1);
    for (i = 0; i < game->cable_count / 2; i++) {
        for (j = 0; j < 2; j++) {
            cable = &game->cables[i * 2 + j];

            cable->index = i * 2 + j;

            cable->cable_base_rect.x = ((i + 1) * column_spacing + j * 48) - 40;
            cable->cable_base_rect.y = 470;
            cable->cable_base_rect.w = 24;
            cable->cable_base_rect.h = 48;

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
}


/*
 * See comment in game.h for more details.
 */
void
sb_game_cleanup(void)
{
    sb_game_type *game = &sb_game;

    free_texture(game->speech_bubble_texture);
    free_texture(game->mug_background_texture);
    free_texture(game->plug_loose_texture);
    free_texture(game->plug_connected_texture);
    free_texture(game->flash_texture);
    free_texture(game->port_texture);
    free_texture(game->console_texture);
}


static sb_gamestate_type sb_game_gamestate = {
    .event_cb = &sb_game_event,
    .update_cb = &sb_game_update,
    .draw_cb = &sb_game_draw,
    .ctx = NULL,
};


/*
 * See comment in game.h for more details.
 */
sb_gamestate_type *
sb_game_get_gamestate(void)
{
    return &sb_game_gamestate;
}

