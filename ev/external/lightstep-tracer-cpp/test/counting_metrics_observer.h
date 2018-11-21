#pragma once

#include <lightstep/metrics_observer.h>
#include <atomic>

namespace lightstep {
struct CountingMetricsObserver : MetricsObserver {
  void OnSpansSent(int num_spans) override { num_spans_sent += num_spans; }

  void OnSpansDropped(int num_spans) override {
    num_spans_dropped += num_spans;
  }

  void OnFlush() override { ++num_flushes; }

  std::atomic<int> num_flushes{0};
  std::atomic<int> num_spans_sent{0};
  std::atomic<int> num_spans_dropped{0};
};
}  // namespace lightstep
