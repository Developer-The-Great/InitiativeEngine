#pragma once
#include "Core.h"
#include <iostream>
#include <spdlog\spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace itv::log
{
		enum class Severity
		{
			debug,
			warning,
			fatal
		};

		void ITV_API init();

		std::shared_ptr<spdlog::logger> ITV_API getLogger();

		spdlog::level::level_enum ITV_API itv2Spdlog(Severity severity);

		template<class... Args, class FormatString>
		void print(Severity severity, const FormatString& format, Args&&... args)
		{
			getLogger()->log(itv2Spdlog(severity), format, std::forward<Args>(args)...);
		}

		#define ITV_LOG(format,...) itv::log::print(itv::log::Severity::debug,format,__VA_ARGS__)
		#define ITV_WARN(format,...) itv::log::print(itv::log::Severity::warning,format,__VA_ARGS__)
		#define ITV_FATAL(format,...) itv::log::print(itv::log::Severity::fatal,format,__VA_ARGS__)

}