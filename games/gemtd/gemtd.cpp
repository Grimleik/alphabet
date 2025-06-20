/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#include "platform_layer.h"
#include "cu_math.h"
#include "entity.h"
#include "input.h"
#include "renderer.h"
#include "logger.h"
#include "memorymanager.h"

struct tile_state
{
	bool clicked;
};

struct GameState
{
	// Renderer::CmdBatch batch;
	size_t tileSize, appron;
	tile_state *tiles;
	size_t w, h;
	std::string banner;
};

static GameState *gameState = nullptr;

extern "C"
{
	void AGE_GameInit(Platform::State *plat, bool reload)
	{
		if (reload)
		{
			gameState = (GameState *)plat->gameState;
			// TODO: PUP Table Function!
			MemoryManager::Instance = (MemoryManager *)plat->pups.memoryManager;
			Input::Instance = (Input *)plat->pups.input;
			Renderer::Instance = (Renderer *)plat->pups.renderer;
			ECS::Instance = (ECS *)plat->pups.ecs;
			gameState->appron = 80;

			gameState->w = Renderer::Instance->settings.width / gameState->tileSize;						// Number of tiles in width
			gameState->w -= 1;
			gameState->h = (Renderer::Instance->settings.height - gameState->appron) / gameState->tileSize; // Number of tiles in height
			// gameState->h -= 1;
		}
		else
		{
			MemoryManager::Instance = (MemoryManager *)plat->pups.memoryManager;
			Input::Instance = (Input *)plat->pups.input;
			Renderer::Instance = (Renderer *)plat->pups.renderer;
			ECS::Instance = (ECS *)plat->pups.ecs;
			plat->gameState = MemoryManager::Instance->PartitionWithArgs<GameState>();
			gameState = (GameState *)plat->gameState;

			gameState->tileSize = 32;
			gameState->appron = 64;
			gameState->w = Renderer::Instance->settings.width / gameState->tileSize;						// Number of tiles in width
			gameState->w -= 1;
			gameState->h = (Renderer::Instance->settings.height - gameState->appron) / gameState->tileSize; // Number of tiles in height
			gameState->tiles = (tile_state *)MemoryManager::Instance->PartitionBlock(sizeof(tile_state) * gameState->w * gameState->h);
			AGE_LOG(LOG_LEVEL::INFO, "Tiles({}, {})", gameState->w, gameState->h);
			plat->isRunning = true;
			plat->dt = 0.0f;
		}
	}

	void AGE_GameUpdate(Platform::State *plat)
	{
		if (Input::Instance->IsKeyDown(Input::KEYS::ESCAPE))
		{
			plat->isRunning = false;
		}

		gameState = (GameState *)plat->gameState;

		// Input:
		int tileX = 0, tileY = 0;
		if (Input::Instance->mouse.lButton)
		{
			tileX = (Input::Instance->mouse.x) / gameState->tileSize;
			tileY = (Input::Instance->mouse.y - gameState->appron) / gameState->tileSize;

			tile_state *tile = gameState->tiles + tileY * gameState->w + tileX;
			if (tile)
			{
				tile->clicked = true;
			}
			else
			{
				AGE_LOG(LOG_LEVEL::ERR, "Failed to click tile. {} {}", tileX, tileY);
			}
		}

		// Rendering:
		Renderer::Instance->PushCmd_ClearScreen({0x00000000}); // Clear screen with black color
		for (int y = 0; y < gameState->h; ++y)
		{
			for (int x = 0; x < gameState->w; ++x)
			{
				tile_state *tile = gameState->tiles + y * gameState->w + x;
				Renderer::CmdRectangle &rect = Renderer::Instance->PushCmd_Rectangle();
				rect.x = (x * gameState->tileSize) + 3;
				rect.y = (Renderer::Instance->settings.height - gameState->appron) - (y * gameState->tileSize);
				rect.w = gameState->tileSize - 3;
				rect.h = gameState->tileSize - 3;
				rect.center = false;
				u32 color = (int)(255 * 0.7) << 16;
				color |= (int)(255 * 0.7) << 8;
				color |= (int)(255 * 0.7);
				if (tile->clicked)
				{
					color = (255 << 16);
				}
				// color |= ((255 * x / gridWidth) << 16);
				// color |= ((255 * y / gridHeight) << 8);
				rect.c = color;
				// rect.c = (u32)(0xFF000000 | (x * 10) | (y * 20)); // Color based on position
				rect.filled = true;
			}
		}
		gameState->banner = std::format("Mouse: {} {} Minesweeper! {} {}", Input::Instance->mouse.x,
										Input::Instance->mouse.y, tileX, tileY);
		Renderer::Instance->PushCmd_Text({0, 0, gameState->banner.c_str(), 14, 0xFF00FFFF, 32.0f}); // Draw text
	}

	void AGE_GameShutdown(Platform::State *plat)
	{
	}
}
