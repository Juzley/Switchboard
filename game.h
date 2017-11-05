#ifndef __GAME_H__
#define __GAME_H__


#include <SDL2/SDL.h>


typedef struct sb_game *sb_game_handle_type;


sb_game_handle_type sb_game_setup(SDL_Renderer *renderer);
void sb_game_cleanup(sb_game_handle_type game);
void sb_game_event(SDL_Event           *e,
                   sb_game_handle_type  game);
void sb_game_update(uint32_t            frametime,
                    sb_game_handle_type game);
void sb_game_draw(SDL_Renderer        *renderer,
                  sb_game_handle_type  game);

#endif /* __GAME_H__ */
