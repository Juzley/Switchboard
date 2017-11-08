#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "gamestate.h"
#include "game.h"
#include "util.h"


#define FONT_NAME "media/carbon.ttf"
#define FONT_SIZE 64


void sb_exit(void);


SDL_Texture *sb_menu_main_new_game_texture;
SDL_Rect     sb_menu_main_new_game_rect;
SDL_Texture *sb_menu_main_exit_texture;
SDL_Rect     sb_menu_main_exit_rect;


static void
sb_menu_main_event (SDL_Event *e,
                    void      *context)
{
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (sb_point_in_rect(e->button.x, e->button.y,
                             &sb_menu_main_new_game_rect)) {
            sb_gamestate_replace_all(sb_game_get_gamestate());
        } else if (sb_point_in_rect(e->button.x, e->button.y,
                                    &sb_menu_main_exit_rect)) {
            sb_exit();
        }
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
    SDL_RenderCopy(renderer, sb_menu_main_new_game_texture, NULL,
                   &sb_menu_main_new_game_rect);
    SDL_RenderCopy(renderer, sb_menu_main_exit_texture, NULL,
                   &sb_menu_main_exit_rect);
}


void
sb_menu_main_setup (SDL_Renderer *renderer)
{
    SDL_Surface  *surf;
    SDL_Color     color = { 255, 255, 255, 255 };
    TTF_Font     *font = TTF_OpenFont(FONT_NAME, FONT_SIZE);

    surf = TTF_RenderText_Blended(font, "New Game", color);
    sb_menu_main_new_game_texture = SDL_CreateTextureFromSurface(renderer,
                                                                 surf);
    SDL_FreeSurface(surf);
    (void)SDL_QueryTexture(sb_menu_main_new_game_texture, NULL, NULL,
                           &sb_menu_main_new_game_rect.w,
                           &sb_menu_main_new_game_rect.h);
    sb_menu_main_new_game_rect.x = 0;
    sb_menu_main_new_game_rect.y = 0;

    surf = TTF_RenderText_Blended(font, "Exit", color);
    sb_menu_main_exit_texture = SDL_CreateTextureFromSurface(renderer,
                                                             surf);
    SDL_FreeSurface(surf);
    (void)SDL_QueryTexture(sb_menu_main_exit_texture, NULL, NULL,
                           &sb_menu_main_exit_rect.w,
                           &sb_menu_main_exit_rect.h);
    sb_menu_main_exit_rect.x = 0;
    sb_menu_main_exit_rect.y = 64;

    TTF_CloseFont(font);
}


void
sb_menu_main_cleanup (void)
{
    free_texture(sb_menu_main_exit_texture);
    sb_menu_main_exit_texture = NULL;

    free_texture(sb_menu_main_new_game_texture);
    sb_menu_main_new_game_texture = NULL;
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


