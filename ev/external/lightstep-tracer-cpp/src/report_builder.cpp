#include "report_builder.h"
#include "utility.h"

namespace lightstep {
//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
ReportBuilder::ReportBuilder(
    const std::string& access_token,
    const std::unordered_map<std::string, opentracing::Value>& tags) {
  // TODO(rnburn): Fill in any core internal_metrics.
  collector::Reporter* reporter = preamble_.mutable_reporter();
  for (const auto& tag : tags) {
    *reporter->mutable_tags()->Add() = ToKeyValue(tag.first, tag.second);
  }
  reporter->set_reporter_id(GenerateId());
  preamble_.mutable_auth()->set_access_token(access_token);
}

//------------------------------------------------------------------------------
// AddSpan
//------------------------------------------------------------------------------
void ReportBuilder::AddSpan(collector::Span&& span) {
  if (reset_next_) {
    pending_.Clear();
    pending_.CopyFrom(preamble_);
    reset_next_ = false;
  }
  *pending_.mutable_spans()->Add() = span;
}

//------------------------------------------------------------------------------
// set_pending_client_dropped_spans
//------------------------------------------------------------------------------
void ReportBuilder::set_pending_client_dropped_spans(uint64_t spans) {
  auto count = pending_.mutable_internal_metrics()->add_counts();
  count->set_name("spans.dropped");
  count->set_int_value(spans);
}
}  // namespace lightstep
