#pragma once

#include <lightstep/transporter.h>
#include <exception>
#include <iostream>
#include <mutex>
#include <vector>
#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
class InMemorySyncTransporter : public SyncTransporter {
 public:
  opentracing::expected<void> Send(
      const google::protobuf::Message& request,
      google::protobuf::Message& response) override;

  std::vector<collector::Span> spans() const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return spans_;
  }

  std::vector<collector::ReportRequest> reports() const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    return reports_;
  }

  void set_should_throw(bool value) {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    should_throw_ = value;
  }

  void set_should_disable(bool value) {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    should_disable_ = value;
  }

 private:
  mutable std::mutex mutex_;
  bool should_throw_ = false;
  bool should_disable_ = false;
  std::vector<collector::ReportRequest> reports_;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
