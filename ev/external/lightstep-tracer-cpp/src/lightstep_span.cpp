#include "lightstep_span.h"
#include "utility.h"

using opentracing::SystemTime;
using opentracing::SystemClock;
using opentracing::SteadyClock;
using opentracing::SteadyTime;

namespace lightstep {
//------------------------------------------------------------------------------
// ComputeStartTimestamps
//------------------------------------------------------------------------------
static std::tuple<SystemTime, SteadyTime> ComputeStartTimestamps(
    const SystemTime& start_system_timestamp,
    const SteadyTime& start_steady_timestamp) {
  // If neither the system nor steady timestamps are set, get the tme from the
  // respective clocks; otherwise, use the set timestamp to initialize the
  // other.
  if (start_system_timestamp == SystemTime() &&
      start_steady_timestamp == SteadyTime()) {
    return std::tuple<SystemTime, SteadyTime>{SystemClock::now(),
                                              SteadyClock::now()};
  }
  if (start_system_timestamp == SystemTime()) {
    return std::tuple<SystemTime, SteadyTime>{
        opentracing::convert_time_point<SystemClock>(start_steady_timestamp),
        start_steady_timestamp};
  }
  if (start_steady_timestamp == SteadyTime()) {
    return std::tuple<SystemTime, SteadyTime>{
        start_system_timestamp,
        opentracing::convert_time_point<SteadyClock>(start_system_timestamp)};
  }
  return std::tuple<SystemTime, SteadyTime>{start_system_timestamp,
                                            start_steady_timestamp};
}

//------------------------------------------------------------------------------
// SetSpanReference
//------------------------------------------------------------------------------
static bool SetSpanReference(
    Logger& logger,
    const std::pair<opentracing::SpanReferenceType,
                    const opentracing::SpanContext*>& reference,
    std::unordered_map<std::string, std::string>& baggage,
    collector::Reference& collector_reference) {
  collector_reference.Clear();
  switch (reference.first) {
    case opentracing::SpanReferenceType::ChildOfRef:
      collector_reference.set_relationship(collector::Reference::CHILD_OF);
      break;
    case opentracing::SpanReferenceType::FollowsFromRef:
      collector_reference.set_relationship(collector::Reference::FOLLOWS_FROM);
      break;
  }
  if (reference.second == nullptr) {
    logger.Warn("Passed in null span reference.");
    return false;
  }
  auto referenced_context =
      dynamic_cast<const LightStepSpanContext*>(reference.second);
  if (referenced_context == nullptr) {
    logger.Warn("Passed in span reference of unexpected type.");
    return false;
  }
  collector_reference.mutable_span_context()->set_trace_id(
      referenced_context->trace_id());
  collector_reference.mutable_span_context()->set_span_id(
      referenced_context->span_id());

  referenced_context->ForeachBaggageItem(
      [&baggage](const std::string& key, const std::string& value) {
        baggage[key] = value;
        return true;
      });

  return true;
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LightStepSpan::LightStepSpan(
    std::shared_ptr<const opentracing::Tracer>&& tracer, Logger& logger,
    Recorder& recorder, opentracing::string_view operation_name,
    const opentracing::StartSpanOptions& options)
    : tracer_{std::move(tracer)},
      logger_{logger},
      recorder_{recorder},
      operation_name_{operation_name} {
  // Set the start timestamps.
  std::tie(start_timestamp_, start_steady_) = ComputeStartTimestamps(
      options.start_system_timestamp, options.start_steady_timestamp);

  // Set any span references.
  std::unordered_map<std::string, std::string> baggage;
  references_.reserve(options.references.size());
  collector::Reference collector_reference;
  for (auto& reference : options.references) {
    if (!SetSpanReference(logger_, reference, baggage, collector_reference)) {
      continue;
    }
    references_.push_back(collector_reference);
  }

  // Set tags.
  for (auto& tag : options.tags) {
    tags_[tag.first] = tag.second;
  }

  // Set opentracing::SpanContext.
  auto trace_id = references_.empty()
                      ? GenerateId()
                      : references_[0].span_context().trace_id();
  auto span_id = GenerateId();
  span_context_ = LightStepSpanContext{trace_id, span_id, std::move(baggage)};
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
LightStepSpan::~LightStepSpan() {
  if (!is_finished_) {
    Finish();
  }
}

//------------------------------------------------------------------------------
// FinishWithOptions
//------------------------------------------------------------------------------
void LightStepSpan::FinishWithOptions(
    const opentracing::FinishSpanOptions& options) noexcept try {
  // Ensure the span is only finished once.
  if (is_finished_.exchange(true)) {
    return;
  }

  auto finish_timestamp = options.finish_steady_timestamp;
  if (finish_timestamp == SteadyTime()) {
    finish_timestamp = SteadyClock::now();
  }

  collector::Span span;

  // Set timing information.
  auto duration = finish_timestamp - start_steady_;
  span.set_duration_micros(
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
  *span.mutable_start_timestamp() = ToTimestamp(start_timestamp_);

  // Set references.
  auto references = span.mutable_references();
  references->Reserve(static_cast<int>(references_.size()));
  for (const auto& reference : references_) {
    *references->Add() = reference;
  }

  // Set tags, logs, and operation name.
  {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    span.set_operation_name(std::move(operation_name_));
    auto tags = span.mutable_tags();
    tags->Reserve(static_cast<int>(tags_.size()));
    for (const auto& tag : tags_) {
      try {
        *tags->Add() = ToKeyValue(tag.first, tag.second);
      } catch (const std::exception& e) {
        logger_.Error(R"(Dropping tag for key ")", tag.first,
                      R"(": )", e.what());
      }
    }
    auto logs = span.mutable_logs();
    for (auto& log : logs_) {
      *logs->Add() = log;
    }
  }

  // Set the span context.
  auto span_context = span.mutable_span_context();
  span_context->set_trace_id(span_context_.trace_id());
  span_context->set_span_id(span_context_.span_id());
  auto baggage = span_context->mutable_baggage();
  span_context_.ForeachBaggageItem(
      [baggage](const std::string& key, const std::string& value) {
        using StringMap = google::protobuf::Map<std::string, std::string>;
        baggage->insert(StringMap::value_type(key, value));
        return true;
      });

  // Record the span
  recorder_.RecordSpan(std::move(span));
} catch (const std::exception& e) {
  logger_.Error("FinishWithOptions failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetOperationName
//------------------------------------------------------------------------------
void LightStepSpan::SetOperationName(
    opentracing::string_view name) noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  operation_name_ = name;
} catch (const std::exception& e) {
  logger_.Error("SetOperationName failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetTag
//------------------------------------------------------------------------------
void LightStepSpan::SetTag(opentracing::string_view key,
                           const opentracing::Value& value) noexcept try {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  tags_[key] = value;
} catch (const std::exception& e) {
  logger_.Error("SetTag failed: ", e.what());
}

//------------------------------------------------------------------------------
// SetBaggageItem
//------------------------------------------------------------------------------
void LightStepSpan::SetBaggageItem(opentracing::string_view restricted_key,
                                   opentracing::string_view value) noexcept {
  span_context_.set_baggage_item(restricted_key, value);
}

//------------------------------------------------------------------------------
// BaggageItem
//------------------------------------------------------------------------------
std::string LightStepSpan::BaggageItem(
    opentracing::string_view restricted_key) const noexcept try {
  return span_context_.baggage_item(restricted_key);
} catch (const std::exception& e) {
  logger_.Error("BaggageItem failed, returning empty string: ", e.what());
  return {};
}

//------------------------------------------------------------------------------
// Log
//------------------------------------------------------------------------------
void LightStepSpan::Log(std::initializer_list<
                        std::pair<opentracing::string_view, opentracing::Value>>
                            fields) noexcept try {
  auto timestamp = SystemClock::now();
  collector::Log log;
  *log.mutable_timestamp() = ToTimestamp(timestamp);
  auto key_values = log.mutable_fields();
  for (const auto& field : fields) {
    try {
      *key_values->Add() = ToKeyValue(field.first, field.second);
    } catch (const std::exception& e) {
      logger_.Error(R"(Failed to log record for key ")", field.first, R"(": )",
                    e.what());
    }
  }
  logs_.emplace_back(std::move(log));
} catch (const std::exception& e) {
  logger_.Error("Log failed: ", e.what());
}
}  // namespace lightstep
