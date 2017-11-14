#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "gamestate.h"
#include "menu_main.h"


#define SB_ENDGAME_PAUSE_TIME 2000


static uint32_t sb_endgame_time;


static void
sb_endgame_event (SDL_Event *e,
                  void      *context)
{
    if ((e->type == SDL_MOUSEBUTTONDOWN ||
         e->type == SDL_KEYDOWN) &&
        sb_endgame_time >= SB_ENDGAME_PAUSE_TIME) {
        sb_gamestate_replace_all(sb_menu_main_get_gamestate());
    }
}


static void
sb_endgame_update (uint32_t  frametime,
                   void     *context)
{
    sb_endgame_time += frametime;
}


static void
sb_endgame_draw (SDL_Renderer *renderer,
                 void         *context)
{
    SDL_Rect rect = { 0, 0, 800, 600 };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &rect);
}


void
sb_endgame_reset (void)
{
    sb_endgame_time = 0;
}


void
sb_endgame_setup (SDL_Renderer *renderer)
{

}


void
sb_endgame_cleanup (void)
{
}


static sb_gamestate_type sb_endgame_gamestate = {
    .event_cb = &sb_endgame_event,
    .update_cb = &sb_endgame_update,
    .draw_cb = &sb_endgame_draw,
    .flags = SB_GAMESTATE_FLAG_DRAW_UNDER
};


sb_gamestate_type *
sb_endgame_get_gamestate (void)
{
    return &sb_endgame_gamestate;
}
