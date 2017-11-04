#include <SDL2/SDL.h>
#include "gamestate.h"
#include "game.h"


#define FONT_NAME
#define FONT_SIZE

static void
sb_menu_main_event (SDL_Event *e,
                    void      *context)
{
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN:
        sb_gamestate_replace_all(sb_game_get_gamestate());
        break;
    default:
        break;
    }
}


static void
sb_menu_main_update (uint32_t  frametime,
                     void     *context)
{
}


static void
sb_menu_main_draw (SDL_Renderer *renderer,
                   void         *context)
{
}


void
sb_menu_main_setup (SDL_Renderer *renderer)
{
    TTF_Font *font = TTF_OpenFont(FONT_NAME, FONT_SIZE);
    SDL_Surface *surf = TTF_RenderText_Solid(font,
                                             "Start Game"

}


void
sb_menu_main_cleanup (void)
{
}


static sb_gamestate_type sb_menu_main_gamestate = {
    .event_cb = &sb_menu_main_event,
    .update_cb = &sb_menu_main_update,
    .draw_cb = &sb_menu_main_draw,
    .ctx = NULL,
};

sb_gamestate_type *
sb_menu_main_get_gamestate (void)
{
    return &sb_menu_main_gamestate;
}


