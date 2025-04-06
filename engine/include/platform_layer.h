#if !defined(PLATFORM_LAYER_H)
/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#define PLATFORM_LAYER_H

#include "core.h"
#include "singleton.h"

namespace Platform
{
	// NOTE: Since we are dynamically loading our 'applications' through a dll
	// and we are allowing ourselevs to do so dynamically, we have to patch up
	// the pointer that we allocated before loading the dll to point to the same
	// singleton instance as the exec in the dll. PUP = PatchUpPointer.
	struct PUPTable 
	{
		ISingleton *memoryManager;
		ISingleton *input;
		ISingleton *renderer;
	};
	struct State
	{
		bool isRunning;
		u64 frame;
		f64 dt;
		f64 totalTime;
		PUPTable pups;
		// NOTE(pf): This is an opaque ptr that the game can bind its memory to.
		// Since we support hot reloading this will be the stable point that the
		// game can associate its internal state to.
		void *gameState;
	};
};

typedef void (*GameInitFunc)(Platform::State *, bool reload);
typedef void (*GameUpdateFunc)(Platform::State *);
typedef void (*GameShutdownFunc)(Platform::State *);

extern "C"
{
	void GAME_API AGE_GameInit(Platform::State *plat, bool reload);
	void GAME_API AGE_GameUpdate(Platform::State *plat);
	void GAME_API AGE_GameShutdown(Platform::State *plat);
}
#endif
