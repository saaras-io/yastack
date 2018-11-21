#include <lightstep/binary_carrier.h>
#include <lightstep/tracer.h>

namespace lightstep {
//------------------------------------------------------------------------------
// Extract
//------------------------------------------------------------------------------
opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
LightStepBinaryReader::Extract(const opentracing::Tracer& tracer) const try {
  auto lightstep_tracer = dynamic_cast<const LightStepTracer*>(&tracer);
  if (lightstep_tracer == nullptr) {
    return opentracing::make_unexpected(opentracing::invalid_carrier_error);
  }
  if (carrier_ == nullptr) {
    return {};
  }
  auto& basic = carrier_->basic_ctx();
  std::unordered_map<std::string, std::string> baggage;
  for (const auto& entry : basic.baggage_items()) {
    baggage.emplace(entry.first, entry.second);
  }
  return lightstep_tracer->MakeSpanContext(basic.trace_id(), basic.span_id(),
                                           std::move(baggage));
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}

//------------------------------------------------------------------------------
// Inject
//------------------------------------------------------------------------------
opentracing::expected<void> LightStepBinaryWriter::Inject(
    const opentracing::Tracer& tracer,
    const opentracing::SpanContext& span_context) const try {
  auto lightstep_tracer = dynamic_cast<const LightStepTracer*>(&tracer);
  if (lightstep_tracer == nullptr) {
    return opentracing::make_unexpected(opentracing::invalid_carrier_error);
  }
  auto trace_span_ids_maybe = lightstep_tracer->GetTraceSpanIds(span_context);
  if (!trace_span_ids_maybe) {
    return opentracing::make_unexpected(trace_span_ids_maybe.error());
  }
  auto& trace_span_ids = *trace_span_ids_maybe;
  carrier_.Clear();
  auto basic = carrier_.mutable_basic_ctx();
  basic->set_trace_id(trace_span_ids[0]);
  basic->set_span_id(trace_span_ids[1]);
  basic->set_sampled(true);

  auto baggage = basic->mutable_baggage_items();

  span_context.ForeachBaggageItem(
      [baggage](const std::string& key, const std::string& value) {
        (*baggage)[key] = value;
        return true;
      });
  return {};
} catch (const std::bad_alloc&) {
  return opentracing::make_unexpected(
      std::make_error_code(std::errc::not_enough_memory));
}
}  // namespace lightstep
