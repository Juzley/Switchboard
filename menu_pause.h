#ifndef __MENU_PAUSE_H__
#define __MENU_PAUSE_H__


#include <SDL2/SDL.h>
#include "gamestate.h"


void sb_menu_pause_setup(SDL_Renderer *renderer);
void sb_menu_pause_cleanup(void);
sb_gamestate_type *sb_menu_pause_get_gamestate(void);


#endif /* __MENU_PAUSE_H__ */
