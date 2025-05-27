#include "wrapper_spdlog.h"
#include "predefined.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <ctime>
#include <cstdarg>
#include <cstdio>
#include <iomanip>
#include <sstream>

std::string wrapper_spdlog_export_name() {
	std::time_t time_today = std::time(nullptr);
	std::tm tm;
	localtime_r(&time_today, &tm);
	std::ostringstream oss;
	oss << std::put_time(&tm, "%Y%m%d_%H%M%S") << "." << SYS_LOGGER_FORMAT;
	return oss.str();
}

static std::shared_ptr<spdlog::logger> wrapper_spdlog() {
	static std::shared_ptr<spdlog::logger> logger = [] {
		auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console->set_pattern("%^[%l]%$ %v");
		auto file = std::make_shared<spdlog::sinks::basic_file_sink_mt>(wrapper_spdlog_export_name(), true);
		file->set_pattern("[%Y-%m-%d-%H:%M:%S] [%l] %v");
		auto local = std::make_shared<spdlog::logger>(SYS_LOGGER_SYSTEM, spdlog::sinks_init_list{console, file});
		local->set_level((spdlog::level::level_enum)SYS_LOGGER_LOGLEVEL);
		spdlog::register_logger(local);
		return local;
	}();

	return logger;
}

extern void log_trace(const char *format, ...) {
	char log_buffer[SYS_LOGGER_SIZE];
	va_list arguments;
	va_start(arguments, format);
	vsnprintf(log_buffer, sizeof(log_buffer), format, arguments);
	wrapper_spdlog()->trace("{}", log_buffer);
	va_end(arguments);
	return;
}

extern void log_debug(const char *format, ...) {
	char log_buffer[SYS_LOGGER_SIZE];
	va_list arguments;
	va_start(arguments, format);
	vsnprintf(log_buffer, sizeof(log_buffer), format, arguments);
	wrapper_spdlog()->debug("{}", log_buffer);
	va_end(arguments);
	return;
}

extern void log_info(const char *format, ...) {
	char log_buffer[SYS_LOGGER_SIZE];
	va_list arguments;
	va_start(arguments, format);
	vsnprintf(log_buffer, sizeof(log_buffer), format, arguments);
	wrapper_spdlog()->info("{}", log_buffer);
	va_end(arguments);
	return;
}

extern void log_warn(const char *format, ...) {
	char log_buffer[SYS_LOGGER_SIZE];
	va_list arguments;
	va_start(arguments, format);
	vsnprintf(log_buffer, sizeof(log_buffer), format, arguments);
	wrapper_spdlog()->warn("{}", log_buffer);
	va_end(arguments);
	return;
}

extern void log_error(const char *format, ...) {
	char log_buffer[SYS_LOGGER_SIZE];
	va_list arguments;
	va_start(arguments, format);
	vsnprintf(log_buffer, sizeof(log_buffer), format, arguments);
	wrapper_spdlog()->error("{}", log_buffer);
	va_end(arguments);
	return;
}

extern void log_critical(const char *format, ...) {
	char log_buffer[SYS_LOGGER_SIZE];
	va_list arguments;
	va_start(arguments, format);
	vsnprintf(log_buffer, sizeof(log_buffer), format, arguments);
	wrapper_spdlog()->critical("{}", log_buffer);
	va_end(arguments);
	return;
}