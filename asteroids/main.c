/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/

#include "platform.h"
#include "game.h"

int main(int argc, char *argv[]) {
    // Main code
    platform_state_t *platform_state = platform_init(game_init);
    platform_start(platform_state, game_logic);
    platform_shutdown(&platform_state, game_shutdown);
    return 0;
}