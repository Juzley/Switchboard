#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__


#include <SDL2/SDL.h>


typedef void (*sb_gamestate_event_fn_type)(SDL_Event *e,
                                           void      *ctx);
typedef void (*sb_gamestate_update_fn_type)(uint32_t  frametime,
                                            void     *ctx);
typedef void (*sb_gamestate_draw_fn_type)(SDL_Renderer *renderer,
                                          void         *ctx);


typedef uint8_t sb_gamestate_flag_type;
#define SB_GAMESTATE_FLAG_DEFAULT    0x00
#define SB_GAMESTATE_FLAG_DRAW_UNDER 0x01

typedef struct sb_gamestate {
    sb_gamestate_event_fn_type   event_cb;
    sb_gamestate_update_fn_type  update_cb;
    sb_gamestate_draw_fn_type    draw_cb;
    void                        *ctx;
    sb_gamestate_flag_type       flags;
} sb_gamestate_type;


void sb_gamestate_push(sb_gamestate_type *state);
void sb_gamestate_replace(sb_gamestate_type *state);
void sb_gamestate_replace_all(sb_gamestate_type *state);
void sb_gamestate_pop(void);
void sb_gamestate_event(SDL_Event *e);
void sb_gamestate_update(uint32_t frametime);
void sb_gamestate_draw(SDL_Renderer *renderer);

#endif /* __GAMESTATE_H__ */
