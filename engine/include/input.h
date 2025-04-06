#if !defined(INPUT_H)
/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#define INPUT_H
#include "core.h"
#include "singleton.h"

class Input : public ISingleton
{
public:
	enum KEY_STATES
	{
		KEY_UP = 0,
		KEY_DOWN = 1,
	};

	// NOTE(pf): Remember to convert the platform specific keys INTO these values.
	enum class KEYS
	{
		ESCAPE = 9,
		W = 25,
		A = 38,
		S = 39,
		D = 40,
		P = 33,
		SPACE = 65,
		LEFT_ARROW = 113,
		RIGHT_ARROW = 114,
		UP_ARROW = 111,
		DOWN_ARROW = 116,
		ENTER = 36,
		BACKSPACE = 22,
		COUNT = 256,
	};

	struct KeyBuffer
	{
		bool keys[(int)KEYS::COUNT];
	};
	static void Create();
	static Input *Instance;

	void SwapInputBuffer();
	bool IsKeyHeld(KEYS key);
	bool IsKeyDown(KEYS key);
	bool IsKeyUp(KEYS key);
	void MarkKey(KEYS key, KEY_STATES state);

private:
	constexpr static u16 NR_OF_INPUT_BUFFERS = 3;
	KeyBuffer buffers[NR_OF_INPUT_BUFFERS];
	i32 activeInputBuffer;
};

#endif