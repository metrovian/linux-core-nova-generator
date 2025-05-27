#include "wrapper_spdlog.h"
#include "predefined.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string wrapper_spdlog_export_name() {
	std::time_t time_today = std::time(nullptr);
	std::tm tm;
	localtime_r(&time_today, &tm);
	std::ostringstream oss;
	oss << "log_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << "." << SYS_LOGGER_FORMAT;
	return oss.str();
}

static std::shared_ptr<spdlog::logger> wrapper_spdlog() {
	static std::shared_ptr<spdlog::logger> logger = [] {
		auto local = std::make_shared<spdlog::logger>(
		    SYS_LOGGER_SYSTEM,
		    spdlog::sinks_init_list{
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
			std::make_shared<spdlog::sinks::basic_file_sink_mt>(wrapper_spdlog_export_name(), true)});

		local->set_level((spdlog::level::level_enum)SYS_LOGGER_LOGLEVEL);
		spdlog::register_logger(local);
		return local;
	}();

	return logger;
}

extern void wrapper_spdlog_trace(const char *msg) {
	wrapper_spdlog()->trace("{}", msg);
	return;
}

extern void wrapper_spdlog_debug(const char *msg) {
	wrapper_spdlog()->debug("{}", msg);
	return;
}

extern void wrapper_spdlog_info(const char *msg) {
	wrapper_spdlog()->info("{}", msg);
	return;
}

extern void wrapper_spdlog_warn(const char *msg) {
	wrapper_spdlog()->warn("{}", msg);
	return;
}

extern void wrapper_spdlog_error(const char *msg) {
	wrapper_spdlog()->error("{}", msg);
	return;
}

extern void wrapper_spdlog_critical(const char *msg) {
	wrapper_spdlog()->critical("{}", msg);
	return;
}