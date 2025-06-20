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

u32 TILE_STATE_COLORS[] =
	{
		(u32)AGE_COLOR(1.0, 1.0, 1.0, 1.0),
		(u32)AGE_COLOR(0.7, 0.7, 0.7, 1.0),
		(u32)AGE_COLOR(1.0, 0.0, 0.0, 1.0),
};

enum TILE_STATE : i32
{
	CLOSED = 0,
	CLICKED = 1,
	BOMB = 2,
};

struct tile_state
{
	i32 state;
};

enum GAME_STATE
{
	MENU,
	PLAYING,
	GAMEOVER,
};

struct GameState
{
	void update();
	void menu();
	void playing();
	void gameover();

	void generate_board();

	i32 tileSize, appron;
	i32 bombs;
	tile_state *tiles;
	i32 w, h;
	GAME_STATE state;
	std::string banner;
};

extern "C"
{
	void AGE_GameInit(Platform::State *plat, bool reload)
	{
		if (reload)
		{
			auto gameState = (GameState *)plat->gameState;
			// TODO: PUP Table Function!
			MemoryManager::Instance = (MemoryManager *)plat->pups.memoryManager;
			Input::Instance = (Input *)plat->pups.input;
			Renderer::Instance = (Renderer *)plat->pups.renderer;
			ECS::Instance = (ECS *)plat->pups.ecs;
			gameState->appron = 80;

			gameState->w = Renderer::Instance->settings.width / gameState->tileSize; // Number of tiles in width
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
			auto gameState = (GameState *)plat->gameState;

			gameState->tileSize = 32;
			gameState->appron = 64;
			gameState->w = Renderer::Instance->settings.width / gameState->tileSize; // Number of tiles in width
			gameState->w -= 1;
			gameState->h = (Renderer::Instance->settings.height - gameState->appron) / gameState->tileSize; // Number of tiles in height
			gameState->tiles = (tile_state *)MemoryManager::Instance->PartitionBlock(sizeof(tile_state) * gameState->w * gameState->h, false);
			gameState->bombs = 10;
			gameState->state = GAME_STATE::MENU;
		}
		plat->isRunning = true;
		plat->dt = 0.0f;
	}
}

void GameState::generate_board()
{
	memset(tiles, 0, w * h * sizeof(tiles[0]));
	for (int i = 0; i < bombs; ++i)
	{
		f32 perc_x = (f32)(rand()) / RAND_MAX;
		f32 perc_y = (f32)(rand()) / RAND_MAX;
		i32 bomb_x = (i32)(w * perc_x);
		i32 bomb_y = (i32)(h * perc_y);
		tile_state *tile = tiles + w * bomb_y + bomb_x;
		tile->state |= TILE_STATE::BOMB;
	}
}

void GameState::menu()
{
	// Input:
	if (Input::Instance->IsKeyDown(Input::KEYS::ENTER))
	{
		state = GAME_STATE::PLAYING;
		generate_board();
	}

	// Rendering:
	Renderer::Instance->PushCmd_ClearScreen({0x00000000}); // Clear screen with black color
	banner = std::format("Press Enter to play!");
	Renderer::Instance->PushCmd_Text({0, Renderer::Instance->settings.height - 64, banner.c_str(), (u32)banner.size(), 0xFF00FFFF, 32.0f}); // Draw text
}

void GameState::playing()
{
	// Input:
	i32 tileX = 0, tileY = 0;
	if (Input::Instance->mouse.lButton)
	{
		tileX = (Input::Instance->mouse.x + 0) / tileSize;
		tileY = (Input::Instance->mouse.y + 32 - appron - 4) / tileSize;

		tile_state *tile = tiles + tileY * w + tileX;
		if (tile && !(tile->state & TILE_STATE::CLICKED))
		{
			// CHECK IF BOMB:
			if (tile->state & TILE_STATE::BOMB)
			{
				state = GAME_STATE::GAMEOVER;
				return;
			}
			tile->state = (int)TILE_STATE::CLICKED;
		}
	}

	// Rendering:
	Renderer::Instance->PushCmd_ClearScreen({0x00000000}); // Clear screen with black color
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			tile_state *tile = tiles + y * w + x;
			int posX = x * tileSize + 4;
			int posY = y * tileSize + appron - 4; // Adjust for
			// if (tile->state & TILE_STATE::BOMB)
			{
				Renderer::CmdRectangle &rect = Renderer::Instance->PushCmd_Rectangle();
				rect.x = posX;
				rect.y = posY;
				rect.w = tileSize - 2;
				rect.h = tileSize - 2;
				rect.center = false;
				rect.c = TILE_STATE_COLORS[tile->state];

				rect.filled = true;
			}
			const char *Numbers[] = {
				"1",
				"2",
				"3",
				"4",
				"5"};

			const u32 NumberColors[] = {
				0xFF0000FF, // 1
				0xFF00FF00, // 2
				0xFFFF0000, // 3
				0xFF00FFFF, // 4
				0xFFFF00FF, // 5
			};
			int bombs = 0;
			if (tile->state == TILE_STATE::CLICKED)
			{
				// Count bombs around this tile:
				for (int dy = -1; dy <= 1; ++dy)
				{
					for (int dx = -1; dx <= 1; ++dx)
					{
						if (dx == 0 && dy == 0)
							continue; // Skip the tile itself
						int nx = x + dx;
						int ny = y + dy;
						if (nx >= 0 && nx < w && ny >= 0 && ny < h)
						{
							tile_state *neighborTile = tiles + ny * w + nx;
							if (neighborTile->state & TILE_STATE::BOMB)
							{
								bombs++;
							}
						}
					}
				}
			}

			if (bombs >= 1 && bombs <= 5)
			{
				// Draw the number of bombs around this tile
				Renderer::CmdText &cmd_txt = Renderer::Instance->PushCmd_Text();
				cmd_txt.text = Numbers[bombs - 1];
				cmd_txt.len = 1;
				cmd_txt.c = NumberColors[bombs - 1];
				// cmd_txt.x = x + (tileSize / 2) - 8;
				// cmd_txt.y = y + (tileSize / 2) - 16;
				cmd_txt.x = posX + (tileSize / 2) - 8;
				cmd_txt.y = posY + (tileSize / 2) - 48;
				cmd_txt.scale = 32.0f;
			}
		}
	}

	Renderer::CmdRectangle &mouse = Renderer::Instance->PushCmd_Rectangle();
	mouse.center = true;
	mouse.filled = false;
	mouse.h = 32;
	mouse.w = 32;
	mouse.x = Input::Instance->mouse.x;
	mouse.y = Input::Instance->mouse.y;

	banner = std::format("Mouse: {} {} Minesweeper! {} {}", Input::Instance->mouse.x,
						 Input::Instance->mouse.y, tileX, tileY);
	auto &cmd_banner = Renderer::Instance->PushCmd_Text(); // Draw text
														   // {0, 0, gameState->banner.c_str() 14, 0xFF00FFFF, 32.0f})
	cmd_banner.x = 20;
	cmd_banner.y = 20;
	cmd_banner.text = banner.c_str();
	cmd_banner.len = (u32)banner.size();
	cmd_banner.c = AGE_WHITE;
	cmd_banner.scale = 32.0f;


	auto &cmd_banner2 = Renderer::Instance->PushCmd_Text(); // Draw text
	cmd_banner2.x = 20;
	cmd_banner2.y = 60;
	cmd_banner2.text = "Click to open tile!";
	cmd_banner2.len = 20;
	cmd_banner2.c = AGE_WHITE;
	cmd_banner2.scale = 32.0f;
}

void GameState::gameover()
{
	// Input:
	if (Input::Instance->IsKeyDown(Input::KEYS::ENTER))
	{
		state = GAME_STATE::MENU;
	}

	// Rendering:
	Renderer::Instance->PushCmd_ClearScreen({0x00000000}); // Clear screen with black color
	banner = std::format("GameOver! Press Enter to exit!");
	Renderer::Instance->PushCmd_Text({0, 0, banner.c_str(), (u32)banner.size(), 0xFF00FFFF, 32.0f}); // Draw text
}

void GameState::update()
{
	switch (state)
	{
	case GAME_STATE::MENU:
		menu();
		break;

	case GAME_STATE::PLAYING:
		playing();
		break;

	case GAME_STATE::GAMEOVER:
		gameover();
		break;
	default:
		break;
	}
}

void AGE_GameUpdate(Platform::State *plat)
{

	// TODO Move ?
	if (Input::Instance->IsKeyDown(Input::KEYS::ESCAPE))
	{
		plat->isRunning = false;
	}

	auto gameState = (GameState *)plat->gameState;
	gameState->update();
}

void AGE_GameShutdown(Platform::State *plat)
{
}
