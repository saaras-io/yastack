#include "utility.h"
#include <opentracing/string_view.h>
#include <opentracing/value.h>
#include <unistd.h>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
//------------------------------------------------------------------------------
// ToTimestamp
//------------------------------------------------------------------------------
google::protobuf::Timestamp ToTimestamp(
    const std::chrono::system_clock::time_point& t) {
  using namespace std::chrono;
  auto nanos = duration_cast<nanoseconds>(t.time_since_epoch()).count();
  google::protobuf::Timestamp ts;
  const uint64_t nanosPerSec = 1000000000;
  ts.set_seconds(nanos / nanosPerSec);
  ts.set_nanos(nanos % nanosPerSec);
  return ts;
}

//------------------------------------------------------------------------------
// GenerateId
//------------------------------------------------------------------------------
uint64_t GenerateId() {
  static thread_local std::mt19937_64 rand_source{std::random_device()()};
  return rand_source();
}

//------------------------------------------------------------------------------
// GetProgramName
//------------------------------------------------------------------------------
std::string GetProgramName() {
  constexpr int path_max = 1024;
  std::unique_ptr<char[]> exe_path(new char[path_max]);
  ssize_t size = ::readlink("/proc/self/exe", exe_path.get(), path_max);
  if (size == -1) {
    return "c++-program";  // Dunno...
  }
  std::string path(exe_path.get(), size);
  size_t lslash = path.rfind('/');
  if (lslash != path.npos) {
    return path.substr(lslash + 1);
  }
  return path;
}

//------------------------------------------------------------------------------
// WriteEscapedString
//------------------------------------------------------------------------------
// The implementation is based off of this answer from StackOverflow:
// https://stackoverflow.com/a/33799784
static void WriteEscapedString(std::ostringstream& writer,
                               opentracing::string_view s) {
  writer << '"';
  for (char c : s) {
    switch (c) {
      case '"':
        writer << R"(\")";
        break;
      case '\\':
        writer << R"(\\)";
        break;
      case '\b':
        writer << R"(\b)";
        break;
      case '\n':
        writer << R"(\n)";
        break;
      case '\r':
        writer << R"(\r)";
        break;
      case '\t':
        writer << R"(\t)";
        break;
      default:
        if ('\x00' <= c && c <= '\x1f') {
          writer << R"(\u)";
          writer << std::hex << std::setw(4) << std::setfill('0')
                 << static_cast<int>(c);
        } else {
          writer << c;
        }
    }
  }
  writer << '"';
}

//------------------------------------------------------------------------------
// ToJson
//------------------------------------------------------------------------------
static void ToJson(std::ostringstream& writer, const opentracing::Value& value);

namespace {
struct JsonValueVisitor {
  std::ostringstream& writer;

  void operator()(bool value) {
    if (value) {
      writer << "true";
    } else {
      writer << "false";
    }
  }

  void operator()(double value) {
    if (std::isnan(value)) {
      writer << R"("NaN")";
    } else if (std::isinf(value)) {
      if (std::signbit(value)) {
        writer << R"("-Inf")";
      } else {
        writer << R"("+Inf")";
      }
    } else {
      writer << value;
    }
  }

  void operator()(int64_t value) { writer << value; }

  void operator()(uint64_t value) { writer << value; }

  void operator()(const std::string& s) { WriteEscapedString(writer, s); }

  void operator()(std::nullptr_t) { writer << "null"; }

  void operator()(const char* s) { WriteEscapedString(writer, s); }

  void operator()(const opentracing::Values& values) {
    writer << '[';
    size_t i = 0;
    for (const auto& value : values) {
      ToJson(writer, value);
      if (++i < values.size()) {
        writer << ',';
      }
    }
    writer << ']';
  }

  void operator()(const opentracing::Dictionary& dictionary) {
    writer << '{';
    size_t i = 0;
    for (const auto& key_value : dictionary) {
      WriteEscapedString(writer, key_value.first);
      writer << ':';
      ToJson(writer, key_value.second);
      if (++i < dictionary.size()) {
        writer << ',';
      }
    }
    writer << '}';
  }
};
}  // anonymous namespace

static void ToJson(std::ostringstream& writer,
                   const opentracing::Value& value) {
  JsonValueVisitor value_visitor{writer};
  apply_visitor(value_visitor, value);
}

static std::string ToJson(const opentracing::Value& value) {
  std::ostringstream writer;
  writer.exceptions(std::ios::badbit | std::ios::failbit);
  ToJson(writer, value);
  return writer.str();
}

//------------------------------------------------------------------------------
// ToKeyValue
//------------------------------------------------------------------------------
namespace {
struct ValueVisitor {
  collector::KeyValue& key_value;
  const opentracing::Value& original_value;

  void operator()(bool value) const { key_value.set_bool_value(value); }

  void operator()(double value) const { key_value.set_double_value(value); }

  void operator()(int64_t value) const { key_value.set_int_value(value); }

  void operator()(uint64_t value) const {
    // There's no uint64_t value type so cast to an int64_t.
    key_value.set_int_value(static_cast<int64_t>(value));
  }

  void operator()(const std::string& s) const { key_value.set_string_value(s); }

  void operator()(std::nullptr_t) const { key_value.set_bool_value(false); }

  void operator()(const char* s) const { key_value.set_string_value(s); }

  void operator()(const opentracing::Values& /*unused*/) const {
    key_value.set_json_value(ToJson(original_value));
  }

  void operator()(const opentracing::Dictionary& /*unused*/) const {
    key_value.set_json_value(ToJson(original_value));
  }
};
}  // anonymous namespace

collector::KeyValue ToKeyValue(opentracing::string_view key,
                               const opentracing::Value& value) {
  collector::KeyValue key_value;
  key_value.set_key(key);
  ValueVisitor value_visitor{key_value, value};
  apply_visitor(value_visitor, value);
  return key_value;
}

//------------------------------------------------------------------------------
// LogReportResponse
//------------------------------------------------------------------------------
void LogReportResponse(Logger& logger, bool verbose,
                       const collector::ReportResponse& response) {
  for (auto& message : response.errors()) {
    logger.Error(message);
  }
  for (auto& message : response.warnings()) {
    logger.Warn(message);
  }
  if (verbose) {
    logger.Info(R"(Report: resp=")", response.ShortDebugString(), R"(")");
    for (auto& message : response.infos()) {
      logger.Info(message);
    }
  }
}
}  // namespace lightstep
