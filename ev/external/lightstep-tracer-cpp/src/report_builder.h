#pragma once

#include <lightstep/tracer.h>
#include <opentracing/value.h>
#include <string>
#include <unordered_map>
#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
// ReportBuilder helps construct lightstep::collector::ReportRequest messages.
// Not thread-safe, thread compatible.
class ReportBuilder {
 public:
  ReportBuilder(
      const std::string& access_token,
      const std::unordered_map<std::string, opentracing::Value>& tags);

  // AddSpan adds the span to the currently-building ReportRequest.
  void AddSpan(collector::Span&& span);

  // num_pending_spans() is the number of pending spans.
  size_t num_pending_spans() const { return pending_.spans_size(); }

  void set_pending_client_dropped_spans(uint64_t spans);

  // pending() returns a mutable object, appropriate for swapping with
  // another ReportRequest object.
  collector::ReportRequest& pending() {
    reset_next_ = true;
    return pending_;
  }

 private:
  bool reset_next_ = true;
  collector::ReportRequest preamble_;
  collector::ReportRequest pending_;
};
}  // namespace lightstep
