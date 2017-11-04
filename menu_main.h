#ifndef __MENU_MAIN_H__
#define __MENU_MAIN_H__


#include <SDL2/SDL.h>
#include "gamestate.h"


void sb_menu_main_setup(SDL_Renderer *renderer);
void sb_menu_main_cleanup(void);
sb_gamestate_type *sb_menu_main_get_gamestate(void);


#endif /* __MENU_MAIN_H__ */
