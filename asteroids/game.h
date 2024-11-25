#if !defined(GAME_H)
/* ========================================================================
   Creator: Grimleik $
   TODO: 
   * Full logic:
      * Collision logic with asteroids and bullets.
      * Add score.
      * Add lives.
      * Add powerups.
      * 
   * Add menu and game over screen.
   * Draw images, using stb_image.h.
   * Add sound.
   * Add particles.
   * Review Code and coding style.
   ========================================================================*/
#define GAME_H

#include "platform.h"

void game_init(platform_state_t *state);
void game_logic(platform_state_t *state);
void game_shutdown(platform_state_t *state);

#endif
