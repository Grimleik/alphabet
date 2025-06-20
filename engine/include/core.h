#if !defined(CORE_H)
/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#define CORE_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef char i8;
typedef short i16;
typedef int i32;
typedef long i64;
typedef float f32;
typedef double f64;

#define KB(x) (x * 1024)
#define MB(x) (KB(x) * 1024)
#define GB(x) (MB(x) * 1024)

// NOTE(pf): Place here ?
#define TARGET_FPS 60
#define TARGET_FRAME_DURATION (1.0 / TARGET_FPS)
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define AGE_Assert(msg) \
	assert(false && (msg));

#define AGE_AssertCheck(pred, msg) \
	assert((pred) && (msg));

#if defined(_WIN32) || defined(_WIN64)
#ifdef GAME_DLL_EXPORT
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __declspec(dllimport)
#endif
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#ifdef GAME_DLL_EXPORT
#define GAME_API __attribute__((visibility("default")))
#else
#define GAME_API
#endif
#else
#error Unsupported Platform.
#endif

template<typename T>
inline T wrap(T value, T max)
{
	static_assert(std::is_integral<T>::value, "only integral values supported.");
	return (value + max) % max;
}

template<typename T>
inline T clamp(T v, T min, T max)
{
	if (v < min)
		return min;
	if (v > max)
		return max;
	return v;
}

#endif
