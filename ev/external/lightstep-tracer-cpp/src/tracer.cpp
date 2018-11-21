#include <lightstep/tracer.h>
#include <lightstep/version.h>
#include <opentracing/string_view.h>
#include <opentracing/value.h>
#include <opentracing/version.h>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "auto_recorder.h"
#include "grpc_transporter.h"
#include "lightstep-tracer-common/collector.pb.h"
#include "lightstep_span_context.h"
#include "lightstep_tracer_impl.h"
#include "logger.h"
#include "manual_recorder.h"
#include "utility.h"

namespace lightstep {
const opentracing::string_view component_name_key = "lightstep.component_name";
const opentracing::string_view collector_service_full_name =
    "lightstep.collector.CollectorService";
const opentracing::string_view collector_method_name = "Report";

//------------------------------------------------------------------------------
// GetDefaultTags
//------------------------------------------------------------------------------
static const std::vector<
    std::pair<opentracing::string_view, opentracing::Value>>&
GetDefaultTags() {
  static const std::vector<
      std::pair<opentracing::string_view, opentracing::Value>>
      default_tags = {{"lightstep.tracer_platform", "c++"},
                      {"lightstep.tracer_platform_version", __cplusplus},
                      {"lightstep.tracer_version", LIGHTSTEP_VERSION},
                      {"lightstep.opentracing_version", OPENTRACING_VERSION}};
  return default_tags;
}

//------------------------------------------------------------------------------
// CollectorServiceFullName
//------------------------------------------------------------------------------
const std::string& CollectorServiceFullName() {
  static std::string name = collector_service_full_name;
  return name;
}

//------------------------------------------------------------------------------
// CollectorMethodName
//------------------------------------------------------------------------------
const std::string& CollectorMethodName() {
  static std::string name = collector_method_name;
  return name;
}

//------------------------------------------------------------------------------
// GetTraceSpanIds
//------------------------------------------------------------------------------
opentracing::expected<std::array<uint64_t, 2>> LightStepTracer::GetTraceSpanIds(
    const opentracing::SpanContext& span_context) const noexcept {
  auto lightstep_span_context =
      dynamic_cast<const LightStepSpanContext*>(&span_context);
  if (lightstep_span_context == nullptr) {
    return opentracing::make_unexpected(
        opentracing::invalid_span_context_error);
  }
  std::array<uint64_t, 2> result = {
      {lightstep_span_context->trace_id(), lightstep_span_context->span_id()}};
  return result;
}

//------------------------------------------------------------------------------
// MakeSpanContext
//------------------------------------------------------------------------------
opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracer::MakeSpanContext(
    uint64_t trace_id, uint64_t span_id,
    std::unordered_map<std::string, std::string>&& baggage) const noexcept try {
  std::unique_ptr<opentracing::SpanContext> result{
      new LightStepSpanContext{trace_id, span_id, std::move(baggage)}};
  return std::move(result);
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}

//------------------------------------------------------------------------------
// MakeThreadedTracer
//------------------------------------------------------------------------------
static std::shared_ptr<LightStepTracer> MakeThreadedTracer(
    std::shared_ptr<Logger> logger, LightStepTracerOptions&& options) {
  std::unique_ptr<SyncTransporter> transporter;
  if (options.transporter != nullptr) {
    transporter = std::unique_ptr<SyncTransporter>{
        dynamic_cast<SyncTransporter*>(options.transporter.get())};
    if (transporter == nullptr) {
      logger->Error(
          "`options.transporter` must be derived from SyncTransporter");
      return nullptr;
    }
    options.transporter.release();
  } else {
    transporter = MakeGrpcTransporter(*logger, options);
  }
  PropagationOptions propagation_options{};
  propagation_options.use_single_key = options.use_single_key_propagation;
  auto recorder = std::unique_ptr<Recorder>{
      new AutoRecorder{*logger, std::move(options), std::move(transporter)}};
  return std::shared_ptr<LightStepTracer>{new LightStepTracerImpl{
      std::move(logger), propagation_options, std::move(recorder)}};
}

//------------------------------------------------------------------------------
// MakeSingleThreadedTracer
//------------------------------------------------------------------------------
static std::shared_ptr<LightStepTracer> MakeSingleThreadedTracer(
    std::shared_ptr<Logger> logger, LightStepTracerOptions&& options) {
  std::unique_ptr<AsyncTransporter> transporter;
  if (options.transporter != nullptr) {
    transporter = std::unique_ptr<AsyncTransporter>{
        dynamic_cast<AsyncTransporter*>(options.transporter.get())};
    if (transporter == nullptr) {
      logger->Error(
          "`options.transporter` must be derived from AsyncTransporter");
      return nullptr;
    }
    options.transporter.release();
  } else {
    logger->Error(
        "`options.transporter` must be set if `options.use_thread` is false");
    return nullptr;
  }
  PropagationOptions propagation_options{};
  propagation_options.use_single_key = options.use_single_key_propagation;
  auto recorder = std::unique_ptr<Recorder>{
      new ManualRecorder{*logger, std::move(options), std::move(transporter)}};
  return std::shared_ptr<LightStepTracer>{new LightStepTracerImpl{
      std::move(logger), propagation_options, std::move(recorder)}};
}

//------------------------------------------------------------------------------
// MakeLightStepTracer
//------------------------------------------------------------------------------
std::shared_ptr<LightStepTracer> MakeLightStepTracer(
    LightStepTracerOptions&& options) noexcept try {
  // Create and configure the logger.
  auto logger = std::make_shared<Logger>(std::move(options.logger_sink));
  try {
    if (options.verbose) {
      logger->set_level(LogLevel::info);
    } else {
      logger->set_level(LogLevel::error);
    }

    // Validate `options`.
    if (options.access_token.empty()) {
      logger->Error("Must provide an access_token!");
      return nullptr;
    }

    // Copy over default tags.
    for (const auto& tag : GetDefaultTags()) {
      options.tags[tag.first] = tag.second;
    }

    // Set the component name if not provided to the program name.
    if (options.component_name.empty()) {
      options.tags.emplace(component_name_key, GetProgramName());
    } else {
      options.tags.emplace(component_name_key, options.component_name);
    }

    if (!options.use_thread) {
      return MakeSingleThreadedTracer(logger, std::move(options));
    }
    return MakeThreadedTracer(logger, std::move(options));
  } catch (const std::exception& e) {
    logger->Error("Failed to construct LightStep Tracer: ", e.what());
    return nullptr;
  }
} catch (const /*spdlog::spdlog_ex&*/ std::exception& e) {
  // Use fprintf to print the error because std::cerr can throw the user
  // configures by calling std::cerr::exceptions.
  std::fprintf(stderr, "Failed to initialize logger: %s\n", e.what());
  return nullptr;
}
}  // namespace lightstep
