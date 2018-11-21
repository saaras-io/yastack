#pragma once

#include <lightstep/transporter.h>
#include <exception>
#include <iostream>
#include <mutex>
#include <vector>
#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
class InMemoryAsyncTransporter : public AsyncTransporter {
 public:
  void Send(const google::protobuf::Message& request,
            google::protobuf::Message& response,
            AsyncTransporter::Callback& callback) override;

  void Write();

  void Fail(std::error_code error);

  const std::vector<collector::ReportRequest>& reports() const {
    return reports_;
  }

  const std::vector<collector::Span>& spans() const { return spans_; }

  void set_should_disable(bool value) { should_disable_ = value; }

 private:
  bool should_disable_ = false;
  const google::protobuf::Message* active_request_;
  google::protobuf::Message* active_response_;
  AsyncTransporter::Callback* active_callback_;
  std::vector<collector::ReportRequest> reports_;
  std::vector<collector::Span> spans_;
};
}  // namespace lightstep
