/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#include "input.h"
#include <memory>
#include "memorymanager.h"

Input *Input::Instance = nullptr;

void Input::Create()
{
	AGE_AssertCheck(Instance == nullptr, "Trying to recreate a singleton instance. Not allowed.");
	Instance = MemoryManager::Instance->PartitionSystem<Input>();
}

void Input::SwapInputBuffer()
{
	activeInputBuffer = wrap(activeInputBuffer + 1, NR_OF_INPUT_BUFFERS);
	memcpy(&buffers[activeInputBuffer],
		   &buffers[wrap(activeInputBuffer - 1, NR_OF_INPUT_BUFFERS)],
		   sizeof(buffers[0].keys[0]) * 256);
}

bool Input::IsKeyHeld(KEYS key)
{
	return buffers[activeInputBuffer].keys[(int)key] != KEY_UP;
}

bool Input::IsKeyDown(KEYS key)
{
	i32 wrapnr = wrap(activeInputBuffer - 1, NR_OF_INPUT_BUFFERS);
	bool b1 = buffers[activeInputBuffer].keys[(int)key] == KEY_DOWN;
	bool b2 = buffers[wrapnr].keys[(int)key] == KEY_UP;
	return b1 && b2;
}

bool Input::IsKeyUp(KEYS key)
{
	return buffers[wrap(activeInputBuffer - 1, NR_OF_INPUT_BUFFERS)].keys[(int)key] == KEY_DOWN &&
		   buffers[activeInputBuffer].keys[(int)key] == KEY_UP;
}

void Input::MarkKey(KEYS key, KEY_STATES state)
{
	buffers[activeInputBuffer].keys[(int)key] = state;
}
