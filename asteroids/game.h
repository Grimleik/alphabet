#if !defined(GAME_H)
/* ========================================================================
   Creator: Grimleik $
   TODO: 
   * Vector math to normalize player speed.
   * Draw images, using stb_image.h.
   * Collision logic with asteroids and bullets.
   * Add menu and game over screen.
   * Add sound.
   * Add particles.
   ========================================================================*/
#define GAME_H

#include "platform.h"


void game_init(platform_state_t *state);
void game_logic(platform_state_t *state);
void game_shutdown(platform_state_t *state);

#endif
