#pragma once

#include <mutex>
#include <stdexcept>
#include <vector>
#include "../src/recorder.h"

namespace lightstep {
// InMemoryRecorder is used for testing only.
class InMemoryRecorder : public Recorder {
 public:
  void RecordSpan(collector::Span&& span) noexcept override {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    spans_.emplace_back(std::move(span));
  }

  std::vector<collector::Span> spans() const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return spans_;
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return spans_.size();
  }

  collector::Span top() const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    if (spans_.empty()) throw std::runtime_error("no spans");
    return spans_.back();
  }

 private:
  mutable std::mutex mutex_;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
