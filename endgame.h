#ifndef __ENDGAME_H__
#define __ENDGAME_H__


#include <SDL2/SDL.h>
#include "gamestate.h"

void sb_endgame_reset(void);
void sb_endgame_setup(SDL_Renderer *renderer);
void sb_endgame_cleanup(void);
sb_gamestate_type *sb_endgame_get_gamestate(void);

#endif /* __ENDGAME_H__ */
