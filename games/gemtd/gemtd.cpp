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

struct GameState
{
	Renderer::CmdBatch batch;
};

static GameState *gameState = nullptr;

extern "C"
{
	void AGE_GameInit(Platform::State *plat, bool reload)
	{
		if (reload)
		{
			gameState = (GameState *)plat->gameState;
		}
		else
		{
			MemoryManager::Instance = (MemoryManager *)plat->pups.memoryManager;
			Input::Instance = (Input *)plat->pups.input;
			Renderer::Instance = (Renderer *)plat->pups.renderer;
			ECS::Instance = (ECS *)plat->pups.ecs;
			plat->gameState = MemoryManager::Instance->PartitionWithArgs<GameState>();
			gameState = (GameState *)plat->gameState;
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
		Renderer::Instance->PushCmd_ClearScreen({0x00000000});			 // Clear screen with black color
		int tileSize = 20;												 // Size of each tile
		int gridWidth = Renderer::Instance->settings.width / tileSize;	 // Number of tiles in width
		int gridHeight = Renderer::Instance->settings.height / tileSize; // Number of tiles in height
		Renderer::CmdBatch &batchCmd = gameState->batch;
		batchCmd.count = gridWidth * gridHeight;
		batchCmd.cmds = MemoryManager::Instance->PartitionBlock(batchCmd.count * sizeof(Renderer::Command));
		for (int y = 0; y < gridHeight; ++y)
		{
			for (int x = 0; x < gridWidth; ++x)
			{
				Renderer::Command *cmd = &((Renderer::Command *)batchCmd.cmds)[y * gridWidth + x];
				*cmd = Renderer::Command();
				cmd->type = Renderer::CMDType::RECTANGLE;
				cmd->rectangle.x = x * tileSize;
				cmd->rectangle.y = y * tileSize;
				cmd->rectangle.w = tileSize - 1;
				cmd->rectangle.h = tileSize - 1;
				cmd->rectangle.c = (u32)(0xFF000000 | (x * 10) | (y * 20)); // Color based on position
				cmd->rectangle.filled = true;
			}
		}
		Renderer::Instance->PushCmd_Text({300, 50, "MINESWEEPER!", 14, 0xFF00FFFF, 32.0f}); // Draw text
	}

	void AGE_GameShutdown(Platform::State *plat)
	{
	}
}
