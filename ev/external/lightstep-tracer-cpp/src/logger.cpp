#include "logger.h"
#include <iostream>
#include <sstream>

namespace lightstep {
//------------------------------------------------------------------------------
// LogDefault
//------------------------------------------------------------------------------
static void LogDefault(LogLevel log_level,
                       opentracing::string_view message) noexcept try {
  std::ostringstream oss;
  switch (log_level) {
    case LogLevel::debug:
      oss << "Debug: ";
      break;
    case LogLevel::info:
      oss << "Info: ";
      break;
    case LogLevel::warn:
      oss << "Warn: ";
      break;
    case LogLevel::error:
      oss << "Error: ";
      break;
    case LogLevel::off:
      /* This should never be reached. */
      return;
  }
  oss << message << '\n';
  std::cerr << oss.str();
} catch (const std::exception& /*e*/) {
  // Ignore errors.
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
Logger::Logger() : logger_sink_{LogDefault} {}

Logger::Logger(
    std::function<void(LogLevel, opentracing::string_view)>&& logger_sink) {
  if (logger_sink) {
    logger_sink_ = std::move(logger_sink);
  } else {
    logger_sink_ = LogDefault;
  }
}

//------------------------------------------------------------------------------
// Log
//------------------------------------------------------------------------------
void Logger::Log(LogLevel level,
                 opentracing::string_view message) noexcept try {
  if (static_cast<int>(level) >= static_cast<int>(level_)) {
    logger_sink_(level, message);
  }
} catch (const std::exception& /*e*/) {
  // Ignore exceptions.
}
}  // namespace lightstep
