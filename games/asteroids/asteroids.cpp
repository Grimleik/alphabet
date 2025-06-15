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

#define ASTEROID_COUNT 4
#define ASTEROID_SPAWN_DELAY 3.0f

// TODO: Data driven instead of hardcoded.
#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 40
#define PLAYER_SPEED 10
#define ASTEROID_WIDTH 10
#define ASTEROID_HEIGHT 10
#define ASTEROID_SPEED 20
#define BULLET_WIDTH 5
#define BULLET_HEIGHT 5
#define BULLET_SPEED 30

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0x00000000
#define COLOR_RED 0xFF0000
#define COLOR_GREEN 0x00FF00
#define COLOR_BLUE 0x0000FF
#define COLOR_YELLOW 0xFFFF00
#define COLOR_BROWN 0xA52A2A
#define COLOR_ORANGE 0xFFA500

#define PLAYER_COLOR COLOR_WHITE
#define ASTEROID_COLOR COLOR_WHITE
#define COLLISION_OUTLINE_COLOR COLOR_GREEN
#define COLLIDED_COLOR COLOR_RED

// struct AsteroidState
// {
// 	entity_manager_t entityManager;
// 	entity_t *player;
// 	i32 asteroidCount;
// 	f32 spawnDelay;
// 	bool paused;
// };

// AsteroidState *get_game_state(Platform::State *state)
// {
// 	return (AsteroidState *)state->gameState;
// }

// void spawn_player(AsteroidState *gameState, i32 x, i32 y)
// {
// 	gameState->player = entity_manager_create_entity(&gameState->entityManager);
// 	gameState->player->type = ET_PLAYER;
// 	transform_t *transform = entity_add_transform_t(&gameState->entityManager, gameState->player);
// 	transform->pos = {(f32)x, (f32)y};
// 	transform->dir = {0, 0};
// 	transform->drag = {0, 0};
// 	transform->vel = {0, 0};
// 	transform->acc = 0.0f;
// 	transform->speed = PLAYER_SPEED;
// 	collision_t *col = entity_add_collision_t(&gameState->entityManager, gameState->player);
// 	col->type = CT_RECTANGLE;
// 	col->width = PLAYER_WIDTH;
// 	col->height = PLAYER_HEIGHT;
// 	health_t *health = entity_add_health_t(&gameState->entityManager, gameState->player, (health_t){3});
// }

// void spawn_asteroid(AsteroidState *gameState)
// {
// 	if (gameState->asteroidCount >= ASTEROID_COUNT ||
// 		gameState->spawnDelay > 0)
// 	{
// 		return;
// 	}
// 	entity_t *asteroid = entity_manager_create_entity(&gameState->entityManager);
// 	asteroid->type = ET_ASTEROID;
// 	transform_t *transform = entity_add_transform_t(&gameState->entityManager, asteroid);
// 	transform->pos = {(f32)(rand() % 800), (f32)(rand() % 600)};
// 	transform->dir = {(f32)(rand() % 5), (f32)(rand() % 5)};
// 	transform->vel = transform->dir * ASTEROID_SPEED;
// 	transform->acc = 0.0f;
// 	transform->drag = {0.0f, 0.0f};
// 	transform->speed = ASTEROID_SPEED;
// 	collision_t *col = entity_add_collision_t(&gameState->entityManager, asteroid);
// 	col->type = CT_RECTANGLE;
// 	col->width = ASTEROID_WIDTH;
// 	col->height = ASTEROID_HEIGHT;
// 	col->mask = CM_PLAYER | CM_BULLET;
// 	health_t *health = entity_add_health_t(&gameState->entityManager, asteroid);
// 	health->health = 1;
// 	gameState->asteroidCount++;
// 	gameState->spawnDelay = ASTEROID_SPAWN_DELAY;
// }

// void spawn_bullet(AsteroidState *gameState)
// {
// 	entity_t *bullet = entity_manager_create_entity(&gameState->entityManager);
// 	bullet->type = ET_BULLET;
// 	transform_t *transform = entity_add_transform_t(&gameState->entityManager, bullet);
// 	transform_t *playerTransform = entity_get_transform_t(&gameState->entityManager, gameState->player);
// 	transform->pos = {playerTransform->pos.x, playerTransform->pos.y};
// 	transform->dir = {playerTransform->dir.x, playerTransform->dir.y};
// 	transform->vel = transform->dir * BULLET_SPEED;
// 	transform->acc = 0.0f;
// 	transform->drag = {0.0f, 0.0f};
// 	collision_t *col = entity_add_collision_t(&gameState->entityManager, bullet);
// 	col->type = CT_RECTANGLE;
// 	col->mask = CM_ASTEROID;
// 	col->width = BULLET_WIDTH * 10;
// 	col->height = BULLET_HEIGHT * 10;
// 	health_t *health = entity_add_health_t(&gameState->entityManager, bullet);
// 	health->health = 1;
// }

// void game_state_shutdown(AsteroidState *gameState)
// {
// 	entity_manager_shutdown(&gameState->entityManager);
// }

// static void entity_modify_health(AsteroidState *gameState, entity_t *entity, health_t *health, i32 amount)
// {
// 	health->health += amount;
// 	if (health->health <= 0)
// 	{
// 		entity_manager_destroy_entity(&gameState->entityManager, entity);
// 		if (entity->type == ET_ASTEROID)
// 		{
// 			gameState->asteroidCount--;
// 		}
// 	}
// }

// extern "C"
// {
// 	void AGE_GameInit(Platform::State *plat, bool reload)
// 	{
// 		MemoryManager::Instance = (MemoryManager *)plat->pups.memoryManager;
// 		Input::Instance = (Input *)plat->pups.input;
// 		Renderer::Instance = (Renderer *)plat->pups.renderer;
// 		plat->isRunning = true;
// 		plat->dt = 0.0f;

// 		plat->gameState = MemoryManager::Instance->Partition<AsteroidState>();
// 		AsteroidState *gameState = get_game_state(plat);
// 		const Renderer::Settings &settings = Renderer::Instance->settings;

// 		entity_manager_init(&gameState->entityManager, 10);
// 		spawn_player(gameState, settings.width / 2, settings.height / 2);

// 		gameState->asteroidCount = 0;
// 	}

// 	void AGE_GameUpdate(Platform::State *plat)
// 	{
// 		if (Input::Instance->IsKeyDown(Input::KEYS::ESCAPE))
// 		{
// 			plat->isRunning = false;
// 		}

// 		AsteroidState *gameState = get_game_state(plat);

// 		if (Input::Instance->IsKeyDown(Input::KEYS::P))
// 		{
// 			gameState->paused = !gameState->paused;
// 		}

// 		if (gameState->paused)
// 		{
// 			return;
// 		}

// 		CLEAR SCREEN:

// 		Renderer::Instance->PushCmd_ClearScreen({0});
// 		LOGIC:
// 		PLAYER:
// 		entity_t *player = gameState->player;
// 		transform_t *playerTransform = entity_get_transform_t(&gameState->entityManager, player);
// 		playerTransform->acc = 0.0f;
// 		vec2f player_movement = {0, 0};
// 		if (Input::Instance->IsKeyHeld(Input::KEYS::W))
// 		{
// 			player_movement.y += 1;
// 		}
// 		if (Input::Instance->IsKeyHeld(Input::KEYS::S))
// 		{
// 			player_movement.y -= 1;
// 		}
// 		if (Input::Instance->IsKeyHeld(Input::KEYS::A))
// 		{
// 			player_movement.x -= 1;
// 		}
// 		if (Input::Instance->IsKeyHeld(Input::KEYS::D))
// 		{
// 			player_movement.x += 1;
// 		}

// 		playerTransform->acc = 0.0f;
// 		if (player_movement.length2() > 0)
// 		{
// 			player_movement = player_movement.normalize();
// 			playerTransform->acc = 1.0f;
// 			playerTransform->dir = player_movement;
// 		}
// 		playerTransform->dir = player_movement;

// 		ASTEROIDS:
// 		if (gameState->asteroidCount < ASTEROID_COUNT)
// 		{
// 			spawn_asteroid(gameState);
// 		}
// 		gameState->spawnDelay -= (f32)plat->dt;

// 		BULLETS:
// 		SPAWN:
// 		if (Input::Instance->IsKeyDown(Input::KEYS::SPACE))
// 		{
// 			TODO:
// 			spawn_bullet(gameState);
// 		}

// 		DRAW COMMAND PUSH:

// 		/* SIMULATION:
// 		 * For now: The only thing that changes the state is if something has a movement component.
// 		 * TODO: Only velocity components are rendered.. should we have a separate component for rendering ?
// 		 * TODO(entityA/entityB): Should we do something else to handle entity destruction during the simulation ?
// 			The good thing about removing them is that they don't linger and cause more events but it
// 			adds this complexity.
// 		 */
// 		for (int i = 1; i < gameState->entityManager.componentArrays[COMPONENT_TRANSFORM].count; ++i)
// 		{
// 			component_array_t *ca_transforms = &gameState->entityManager.componentArrays[COMPONENT_TRANSFORM];
// 			int velLU = ca_transforms->lookUp[i];
// 			entity_t *entity = &gameState->entityManager.entities[velLU];
// 			if (!entity->id)
// 				continue;

// 			transform_t *transform = entity_get_transform_t(&gameState->entityManager, entity);

// 			u32 color = PLAYER_COLOR;
// 			i32 width = PLAYER_WIDTH;
// 			i32 height = PLAYER_HEIGHT;
// 			switch (entity->type)
// 			{
// 			case ET_PLAYER:
// 				color = PLAYER_COLOR;
// 				width = PLAYER_WIDTH;
// 				height = PLAYER_HEIGHT;
// 				break;
// 			case ET_ASTEROID:
// 				color = ASTEROID_COLOR;
// 				width = ASTEROID_WIDTH;
// 				height = ASTEROID_HEIGHT;
// 				break;
// 			case ET_BULLET:
// 				color = COLOR_YELLOW;
// 				width = BULLET_WIDTH;
// 				height = BULLET_HEIGHT;
// 			default:
// 				break;
// 			}
// 			f32 &dt = plat->dt;
// 			f32 &acc = transform->acc;
// 			vec2f &vel = transform->vel;
// 			vec2f &dir = transform->dir;
// 			vec2f &pos = transform->pos;
// 			vel = vel + (dir * acc * transform->speed * dt);
// 			transform->pos = transform->pos + transform->vel * dt;
// 			vel.x *= 1.0f - (transform->drag.x * dt);
// 			vel.y *= 1.0f - (transform->drag.y * dt);

// 			const Renderer::Settings &settings = Renderer::Instance->settings;
// 			if (pos.x > settings.width)
// 			{
// 				pos.x -= settings.width;
// 			}
// 			if (pos.x < 0)
// 			{
// 				pos.x += settings.width;
// 			}

// 			if (pos.y > settings.height)
// 			{
// 				pos.y -= settings.height;
// 			}

// 			if (pos.y < 0)
// 			{
// 				pos.y += settings.height;
// 			}

// 			NOTE: Render AFTER positional updates.
// 			i32 lineLength = width;
// 			Renderer::Instance->PushCmd_Rectangle({.x = (i32)transform->pos.x,
// 												   .y = (i32)transform->pos.y,
// 												   .w = (i32)width,
// 												   .h = (i32)height,
// 												   .c = COLOR_WHITE});

// 			Renderer::Instance->PushCmd_Line({.x1 = (i32)transform->pos.x,
// 											  .y1 = (i32)transform->pos.y,
// 											  .x2 = (i32)(transform->pos.x + transform->dir.x * lineLength),
// 											  .y2 = (i32)(transform->pos.y + transform->dir.y * lineLength),
// 											  .c = COLOR_WHITE});
// 		}

// 		COLLISION:

// 		component_array_t *ca_collision = &gameState->entityManager.componentArrays[COMPONENT_COLLISION];
// 		for (int i = 1; i < ca_collision->count; ++i)
// 		{
// 			entity_t *entityA = entity_manager_get_entity(&gameState->entityManager, ca_collision->lookUp[i]);
// 			if (!entityA->id)
// 				continue;

// 			collision_t *colA = entity_get_collision_t(&gameState->entityManager, entityA);
// 			bool collision = false;
// 			for (int j = i + 1; j < ca_collision->count; ++j)
// 			{
// 				entity_t *entityB = entity_manager_get_entity(&gameState->entityManager, ca_collision->lookUp[j]);
// 				if (!entityB->id)
// 					continue;

// 				transform_t *tA = entity_get_transform_t(&gameState->entityManager, entityA);
// 				collision_t *colB = entity_get_collision_t(&gameState->entityManager, entityB);

// 				transform_t *tB = entity_get_transform_t(&gameState->entityManager, entityB);

// 				i8 mask = ((colA->mask & (1 << entityB->type)) << 1) | (colB->mask & (1 << entityA->type));
// 				bool collision = mask && check_collision(colA, tA, colB, tB);
// 				Check A
// 				if ((mask & 2) && collision)
// 				{
// 					check_collision(colA, tA, colB, tB);

// 					Renderer::Instance->PushCmd_Rectangle({.x = (i32)tA->pos.x,
// 														   .y = (i32)tA->pos.y,
// 														   .w = colA->width + colB->width,
// 														   .h = colA->height + colB->height,
// 														   .c = COLLISION_OUTLINE_COLOR});

// 					colA->colliding = true;
// 					collision = true;
// 					gameState->paused = true;
// 					printf("Entity %d colliding with %d\n", entityA->id, entityB->id);
// 					health_t *healthA = entity_get_health_t(&gameState->entityManager, entityA);
// 					health_t *healthB = entity_get_health_t(&gameState->entityManager, entityB);
// 					if (healthA)
// 						entity_modify_health(gameState, entityA, healthA, -1);
// 					if (healthB)
// 						entity_modify_health(gameState, entityB, healthB, -1);
// 				}

// 				Check B
// 				if ((mask & 1) && collision)
// 				{
// 					printf("Entity %d colliding with %d\n", entityB->id, entityA->id);
// 					colB->colliding = true;
// 					gameState->paused = true;

// 					Renderer::Instance->PushCmd_Rectangle({.x = (i32)tB->pos.x,
// 														   .y = (i32)tB->pos.y,
// 														   .w = colA->width + colB->width,
// 														   .h = colA->height + colB->height,
// 														   .c = COLLISION_OUTLINE_COLOR});

// 					health_t *healthA = entity_get_health_t(&gameState->entityManager, entityA);
// 					health_t *healthB = entity_get_health_t(&gameState->entityManager, entityB);
// 					if (healthA)
// 						entity_modify_health(gameState, entityA, healthA, -1);
// 					if (healthB)
// 						entity_modify_health(gameState, entityB, healthB, -1);
// 				}

// 				if (collision)
// 				{
// 					Renderer::Instance->PushCmd_Rectangle({.x = (i32)tA->pos.x,
// 														   .y = (i32)tA->pos.y,
// 														   .w = colA->width + colB->width,
// 														   .h = colA->height + colB->height,
// 														   .c = COLOR_YELLOW});
// 					Renderer::Instance->PushCmd_Rectangle({.x = (i32)tB->pos.x,
// 														   .y = (i32)tB->pos.y,
// 														   .w = colA->width + colB->width,
// 														   .h = colA->height + colB->height,
// 														   .c = COLOR_YELLOW});
// 				}
// 			}
// 			if (!collision)
// 			{
// 				colA->colliding = false;
// 			}
// 		}

// 		!SIMULATION

// 		Renderer::Instance->PushCmd_Text({.x = 10,
// 										  .y = 10,
// 										  .text = "Hello World",
// 										  .len = 11,
// 										  .c = COLOR_WHITE});
// 	}

// 	void AGE_GameShutdown(Platform::State *plat)
// 	{
// 		game_state_shutdown(get_game_state(plat));
// 	}
// }
