#include "lightstep_tracer_impl.h"
#include "lightstep_span.h"
#include "lightstep_span_context.h"

namespace lightstep {

//------------------------------------------------------------------------------
// InjectImpl
//------------------------------------------------------------------------------
template <class Carrier>
static opentracing::expected<void> InjectImpl(
    const PropagationOptions& propagation_options,
    const opentracing::SpanContext& span_context, Carrier& writer) {
  auto lightstep_span_context =
      dynamic_cast<const LightStepSpanContext*>(&span_context);
  if (lightstep_span_context == nullptr) {
    return opentracing::make_unexpected(
        opentracing::invalid_span_context_error);
  }
  return lightstep_span_context->Inject(propagation_options, writer);
}

//------------------------------------------------------------------------------
// ExtractImpl
//------------------------------------------------------------------------------
template <class Carrier>
opentracing::expected<std::unique_ptr<opentracing::SpanContext>> ExtractImpl(
    const PropagationOptions& propagation_options, Carrier& reader) {
  LightStepSpanContext* lightstep_span_context;
  try {
    lightstep_span_context = new LightStepSpanContext{};
  } catch (const std::bad_alloc&) {
    return opentracing::make_unexpected(
        make_error_code(std::errc::not_enough_memory));
  }
  std::unique_ptr<opentracing::SpanContext> span_context(
      lightstep_span_context);
  auto result = lightstep_span_context->Extract(propagation_options, reader);
  if (!result) {
    return opentracing::make_unexpected(result.error());
  }
  if (!*result) {
    span_context.reset();
  }
  return std::move(span_context);
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LightStepTracerImpl::LightStepTracerImpl(
    const PropagationOptions& propagation_options,
    std::unique_ptr<Recorder>&& recorder) noexcept
    : logger_{std::make_shared<Logger>()},
      propagation_options_{propagation_options},
      recorder_{std::move(recorder)} {}

LightStepTracerImpl::LightStepTracerImpl(
    std::shared_ptr<Logger> logger,
    const PropagationOptions& propagation_options,
    std::unique_ptr<Recorder>&& recorder) noexcept
    : logger_{std::move(logger)},
      propagation_options_{propagation_options},
      recorder_{std::move(recorder)} {}

//------------------------------------------------------------------------------
// StartSpanWithOptions
//------------------------------------------------------------------------------
std::unique_ptr<opentracing::Span> LightStepTracerImpl::StartSpanWithOptions(
    opentracing::string_view operation_name,
    const opentracing::StartSpanOptions& options) const noexcept try {
  return std::unique_ptr<opentracing::Span>{new LightStepSpan{
      shared_from_this(), *logger_, *recorder_, operation_name, options}};
} catch (const std::exception& e) {
  logger_->Error("StartSpanWithOptions failed: ", e.what());
  return nullptr;
}

//------------------------------------------------------------------------------
// Inject
//------------------------------------------------------------------------------
opentracing::expected<void> LightStepTracerImpl::Inject(
    const opentracing::SpanContext& span_context, std::ostream& writer) const {
  return InjectImpl(propagation_options_, span_context, writer);
}

opentracing::expected<void> LightStepTracerImpl::Inject(
    const opentracing::SpanContext& span_context,
    const opentracing::TextMapWriter& writer) const {
  return InjectImpl(propagation_options_, span_context, writer);
}

opentracing::expected<void> LightStepTracerImpl::Inject(
    const opentracing::SpanContext& span_context,
    const opentracing::HTTPHeadersWriter& writer) const {
  return InjectImpl(propagation_options_, span_context, writer);
}

//------------------------------------------------------------------------------
// Extract
//------------------------------------------------------------------------------
opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracerImpl::Extract(std::istream& reader) const {
  return ExtractImpl(propagation_options_, reader);
}

opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracerImpl::Extract(const opentracing::TextMapReader& reader) const {
  return ExtractImpl(propagation_options_, reader);
}

opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepTracerImpl::Extract(
    const opentracing::HTTPHeadersReader& reader) const {
  return ExtractImpl(propagation_options_, reader);
}

//------------------------------------------------------------------------------
// Flush
//------------------------------------------------------------------------------
bool LightStepTracerImpl::Flush() noexcept {
  return recorder_->FlushWithTimeout(std::chrono::hours(24));
}

//------------------------------------------------------------------------------
// Close
//------------------------------------------------------------------------------
void LightStepTracerImpl::Close() noexcept { Flush(); }
}  // namespace lightstep
