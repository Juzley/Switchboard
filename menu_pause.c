#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "gamestate.h"
#include "util.h"
#include "menu_main.h"


#define FONT_NAME "media/carbon.ttf"
#define FONT_SIZE 64


SDL_Texture *sb_menu_pause_resume_texture;
SDL_Rect     sb_menu_pause_resume_rect;
SDL_Texture *sb_menu_pause_exit_texture;
SDL_Rect     sb_menu_pause_exit_rect;


static void
sb_menu_pause_event (SDL_Event *e,
                     void      *context)
{
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (sb_point_in_rect(e->button.x, e->button.y,
                             &sb_menu_pause_resume_rect)) {
            sb_gamestate_pop();
        } else if (sb_point_in_rect(e->button.x, e->button.y,
                                    &sb_menu_pause_exit_rect)) {
            sb_gamestate_replace_all(sb_menu_main_get_gamestate());
        }
        break;

    default:
        break;
    }
}


static void
sb_menu_pause_update (uint32_t  frametime,
                      void     *context)
{
}


static void
sb_menu_pause_draw (SDL_Renderer *renderer,
                    void         *context)
{
    SDL_Rect rect = { 0, 0, 800, 600 };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderCopy(renderer, sb_menu_pause_resume_texture, NULL,
                   &sb_menu_pause_resume_rect);
    SDL_RenderCopy(renderer, sb_menu_pause_exit_texture, NULL,
                   &sb_menu_pause_exit_rect);
}


void
sb_menu_pause_setup (SDL_Renderer *renderer)
{
    SDL_Surface *surf;
    SDL_Color    color = { 255, 255, 255, 255 };
    TTF_Font    *font = TTF_OpenFont(FONT_NAME, FONT_SIZE);

    surf = TTF_RenderText_Blended(font, "Resume", color);
    sb_menu_pause_resume_texture = SDL_CreateTextureFromSurface(renderer,
                                                                surf);
    SDL_FreeSurface(surf);
    (void)SDL_QueryTexture(sb_menu_pause_resume_texture, NULL, NULL,
                           &sb_menu_pause_resume_rect.w,
                           &sb_menu_pause_resume_rect.h);
    sb_menu_pause_resume_rect.x = 0;
    sb_menu_pause_resume_rect.y = 0;


    surf = TTF_RenderText_Blended(font, "Exit", color);
    sb_menu_pause_exit_texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    (void)SDL_QueryTexture(sb_menu_pause_exit_texture, NULL, NULL,
                           &sb_menu_pause_exit_rect.w,
                           &sb_menu_pause_exit_rect.h);
    sb_menu_pause_exit_rect.x = 0;
    sb_menu_pause_exit_rect.y = FONT_SIZE;

    TTF_CloseFont(font);
}


void
sb_menu_pause_cleanup (void)
{
    free_texture(sb_menu_pause_resume_texture);
    sb_menu_pause_resume_texture = NULL;

    free_texture(sb_menu_pause_exit_texture);
    sb_menu_pause_exit_texture = NULL;
}


static sb_gamestate_type sb_menu_pause_gamestate = {
    .event_cb = &sb_menu_pause_event,
    .update_cb = &sb_menu_pause_update,
    .draw_cb = &sb_menu_pause_draw,
    .ctx = NULL,
    .flags = SB_GAMESTATE_FLAG_DRAW_UNDER
};


sb_gamestate_type *
sb_menu_pause_get_gamestate (void)
{
    return &sb_menu_pause_gamestate;
}

