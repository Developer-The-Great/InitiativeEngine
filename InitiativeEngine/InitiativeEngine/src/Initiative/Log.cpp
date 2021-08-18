#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace itv::log
{
	static std::shared_ptr<spdlog::logger> sCoreLogger = spdlog::stdout_color_mt("Console");

	void init()
	{
		sCoreLogger->set_pattern("[%^%=7l%$] : %v");
		sCoreLogger->set_level(spdlog::level::trace);
	}

	std::shared_ptr<spdlog::logger> getLogger()
	{
		return sCoreLogger;
	}

	spdlog::level::level_enum itv2Spdlog(Severity severity)
	{
		switch (severity)
		{
		case Severity::debug:
			return spdlog::level::debug;
		case Severity::warning:
			return spdlog::level::warn;
		case Severity::fatal:
			return spdlog::level::critical;
		default:
			return spdlog::level::off;
		}
	}

}

