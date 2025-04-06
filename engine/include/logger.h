#if !defined(LOGGER_H)
/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#define LOGGER_H

#include <iostream>
#include <string>
#include <string_view>
#include <format>

#define LOG_ENABLED 0

enum class LOG_LEVEL
{
	ERR,
	WARN,
	INFO,
	DEBUG,
};

constexpr const char *LOG_LEVEL_STR[] = {
	"ERROR",
	"WARN",
	"INFO",
	"DEBUG",
};

constexpr const char *LOG_LEVEL_STR_COL[] = {
	"\033[31m",
	"\033[33m",
	"\033[0m",
	"\033[32m",
};

#if LOG
template <typename... Args>
inline void AGE_LOG(LOG_LEVEL ll, const std::string &msg, Args &&...args)
{
	std::string formattedMsg = std::vformat(msg, std::make_format_args(std::forward<Args &>(args)...));

	std::cout << LOG_LEVEL_STR_COL[(int)ll]
			  << "[" << LOG_LEVEL_STR[(int)ll] << "] "
			  << LOG_LEVEL_STR_COL[(int)LOG_LEVEL::INFO]
			  << formattedMsg
			  << std::endl;
}
#else

template <typename... Args>
inline void AGE_LOG(LOG_LEVEL ll, const std::string &msg, Args &&...args)
{
}

#endif
#endif
