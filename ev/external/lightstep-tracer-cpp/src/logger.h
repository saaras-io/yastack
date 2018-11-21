#pragma once

#include <lightstep/tracer.h>
#include <exception>
#include <sstream>

namespace lightstep {
inline void Concatenate(std::ostringstream& /*oss*/) {}

template <class TFirst, class... TRest>
void Concatenate(std::ostringstream& oss, const TFirst& tfirst,
                 const TRest&... trest) {
  oss << tfirst;
  Concatenate(oss, trest...);
}

class Logger {
 public:
  Logger();

  explicit Logger(
      std::function<void(LogLevel, opentracing::string_view)>&& logger_sink);

  void Log(LogLevel level, opentracing::string_view message) noexcept;

  void log(LogLevel level, const char* message) noexcept {
    Log(level, opentracing::string_view{message});
  }

  template <class... Tx>
  void Log(LogLevel level, const Tx&... tx) noexcept try {
    std::ostringstream oss;
    Concatenate(oss, tx...);
    Log(level, opentracing::string_view{oss.str()});
  } catch (const std::exception& /*e*/) {
    // Ignore exceptions.
  }

  template <class... Tx>
  void Debug(const Tx&... tx) noexcept {
    Log(LogLevel::debug, tx...);
  }

  template <class... Tx>
  void Info(const Tx&... tx) noexcept {
    Log(LogLevel::info, tx...);
  }

  template <class... Tx>
  void Warn(const Tx&... tx) noexcept {
    Log(LogLevel::warn, tx...);
  }

  template <class... Tx>
  void Error(const Tx&... tx) noexcept {
    Log(LogLevel::error, tx...);
  }

  void set_level(LogLevel level) noexcept { level_ = level; }

 private:
  std::function<void(LogLevel, opentracing::string_view)> logger_sink_;
  LogLevel level_ = LogLevel::error;
};
}  // namespace lightstep
