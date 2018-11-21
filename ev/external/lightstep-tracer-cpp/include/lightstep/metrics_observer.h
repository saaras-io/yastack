#pragma once

namespace lightstep {
// MetricsObserver can be used to track LightStep tracer events.
class MetricsObserver {
 public:
  virtual ~MetricsObserver() = default;

  // OnSpansSent records spans transported.
  virtual void OnSpansSent(int /*num_spans*/) {}

  // OnSpansDropped records spans dropped.
  virtual void OnSpansDropped(int /*num_spans*/) {}

  // OnFlush records flush events by the recorder.
  virtual void OnFlush() {}
};
}  // namespace lightstep
