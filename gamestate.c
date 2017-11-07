#include <assert.h>
#include "gamestate.h"

#define MAX_GAMESTATES 16

typedef struct sb_gamestate_mgr {
    sb_gamestate_type gamestate_stack[MAX_GAMESTATES];
    size_t            gamestate_count;
} sb_gamestate_mgr_type;


#define TOP_GAMESTATE                                                         \
    (sb_gamestate_mgr.gamestate_stack[sb_gamestate_mgr.gamestate_count])


static sb_gamestate_mgr_type sb_gamestate_mgr;


void
sb_gamestate_push (sb_gamestate_type *state)
{
    assert(sb_gamestate_mgr.gamestate_count < MAX_GAMESTATES);
    sb_gamestate_mgr.gamestate_stack[
        sb_gamestate_mgr.gamestate_count++] = *state;
}


void
sb_gamestate_pop (void)
{
    assert(sb_gamestate_mgr.gamestate_count > 0);
    sb_gamestate_mgr.gamestate_count--;
}


void
sb_gamestate_event (SDL_Event *e)
{
    assert(sb_gamestate_mgr.gamestate_count > 0);
    TOP_GAMESTATE.event_cb(e, TOP_GAMESTATE.ctx);
}


void
sb_gamestate_update (uint32_t frametime)
{
    assert(sb_gamestate_mgr.gamestate_count > 0);
    TOP_GAMESTATE.update_cb(frametime, TOP_GAMESTATE.ctx);
}


void
sb_gamestate_draw (SDL_Renderer *renderer)
{
    assert(sb_gamestate_mgr.gamestate_count > 0);
    TOP_GAMESTATE.draw_cb(renderer, TOP_GAMESTATE.ctx);
}




