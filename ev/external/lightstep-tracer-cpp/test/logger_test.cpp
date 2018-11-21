#include "../src/logger.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>

using namespace lightstep;

TEST_CASE("logger") {
  LogLevel logged_level = LogLevel::off;
  std::string logged_message;
  Logger logger{[&](LogLevel level, opentracing::string_view message) {
    logged_level = level;
    logged_message = message;
  }};
  logger.set_level(LogLevel::warn);

  SECTION(
      "If log level is less than the logger's level, the message is ignored") {
    logger.Info("t");
    CHECK(logged_message.empty());
  }

  SECTION(
      "If log level is greater than or equal to the logger's message, the "
      "logger sink receive's the message") {
    logger.Warn("t1");
    CHECK(logged_level == LogLevel::warn);
    CHECK(logged_message == "t1");

    logger.Error("t2");
    CHECK(logged_level == LogLevel::error);
    CHECK(logged_message == "t2");
  }

  SECTION("Multiple arguments are concatenated together") {
    logger.Warn("a", "bc");
    CHECK(logged_message == "abc");

    logger.Warn("a", "bc", 123);
    CHECK(logged_message == "abc123");
  }
}
