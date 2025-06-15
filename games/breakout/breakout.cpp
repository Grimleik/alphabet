/* ========================================================================
	Creator: Grimleik $
	NOTES: Fuck STL, roll your own.
========================================================================*/
#include "platform_layer.h"
#include "cu_math.h"
#include "entity.h"
#include "input.h"
#include "renderer.h"
#include "logger.h"
#include "memorymanager.h"
#include "containers.h"

extern "C"
{
	void AGE_GameInit(Platform::State *plat, bool reload)
	{
		MemoryManager::Instance = (MemoryManager *)plat->pups.memoryManager;
		Input::Instance = (Input *)plat->pups.input;
		Renderer::Instance = (Renderer *)plat->pups.renderer;
		ECS::Instance = (ECS *)plat->pups.ecs;
		if (!reload)
		{
			plat->isRunning = true;
			plat->dt = 0.0f;
			// StaticArray<EntityID> entities(1000);
			// entities[2] = 1;
			// HashMap<u32, i32> map(1000);
			// HashMap<u32, AGEString> map2(1000);
			// map[1] = 1;
			// map[2] = 2;
			// map2[1] = "2";
			// map2[2] = "3";
			// entities[0] = ECS::Instance->CreateEntity<Transform, Physics, Collision>(
			// 	Transform{vec2f(0.0f, 0.0f)},
			// 	Physics{vec2f(0.0f, 0.0f), vec2f(0.0f, 0.0f), vec2f(1.0f, 1.0f), 1.0f, 1.0f},
			// 	Collision{1, COLLISION_TYPE::CT_RECTANGLE, 10, 10});

			// entities[1] = ECS::Instance->CreateEntity<Transform, Physics, Collision>(
			// 	Transform{vec2f(0.0f, 0.0f)},
			// 	Physics{vec2f(0.0f, 0.0f), vec2f(0.0f, 0.0f), vec2f(1.0f, 1.0f), 1.0f, 1.0f},
			// 	Collision{1, COLLISION_TYPE::CT_RECTANGLE, 10, 10});

			// entities[2] = ECS::Instance->CreateEntity<Transform, Physics, Collision>(
			// 	Transform{vec2f(0.0f, 0.0f)},
			// 	Physics{vec2f(0.0f, 0.0f), vec2f(0.0f, 0.0f), vec2f(1.0f, 1.0f), 1.0f, 1.0f},
			// 	Collision{1, COLLISION_TYPE::CT_RECTANGLE, 10, 10});

			// AGE_LOG(LOG_LEVEL::DEBUG, "Entities[0]: {}", entities[0]);
			// AGE_LOG(LOG_LEVEL::DEBUG, "Entities[1]: {}", entities[1]);
			// AGE_LOG(LOG_LEVEL::DEBUG, "Entities[2]: {}", entities[2]);

			// AGE_LOG(LOG_LEVEL::DEBUG, "Map[1]: {} and Map2[1]: {}", map[1], map2[1].c_str());
			// AGE_LOG(LOG_LEVEL::DEBUG, "Map[2]: {} and Map2[2]: {}", map[2], map2[2].c_str());
			// AGE_LOG(LOG_LEVEL::DEBUG, "Map[3]: {} and Map2[3]: {}", map[3], map2[3].c_str());
		}
	}

	void AGE_GameUpdate(Platform::State *plat)
	{
		if (Input::Instance->IsKeyDown(Input::KEYS::ESCAPE))
		{
			plat->isRunning = false;
		}
	}

	void AGE_GameShutdown(Platform::State *plat)
	{
	}
}
