#ifndef __GAME_H__
#define __GAME_H__


#include <SDL2/SDL.h>
#include "gamestate.h"


void sb_game_setup(SDL_Renderer *renderer);
void sb_game_cleanup(void);
sb_gamestate_type *sb_game_get_gamestate(void);

#endif /* __GAME_H__ */
