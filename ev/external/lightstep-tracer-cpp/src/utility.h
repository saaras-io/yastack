#pragma once

#include <opentracing/string_view.h>
#include <opentracing/value.h>
#include <chrono>
#include <string>
#include "lightstep-tracer-common/collector.pb.h"
#include "logger.h"

namespace lightstep {
// Convert a std::chrono::system_clock::time_point to the time value used
// by protobuf.
google::protobuf::Timestamp ToTimestamp(
    const std::chrono::system_clock::time_point& t);

// Generates a random uint64_t.
uint64_t GenerateId();

// Attempts to determine the name of the executable invoked.  Returns
// "c++-program" if unsuccessful.
std::string GetProgramName();

// Converts an OpenTracing key-value pair to the key-value pair used in the
// protobuf data structures.
collector::KeyValue ToKeyValue(opentracing::string_view key,
                               const opentracing::Value& value);

// Logs any information returned by the collector.
void LogReportResponse(Logger& logger, bool verbose,
                       const collector::ReportResponse& response);
}  // namespace lightstep
