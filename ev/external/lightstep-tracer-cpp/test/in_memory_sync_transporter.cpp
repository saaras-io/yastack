#include "in_memory_sync_transporter.h"

namespace lightstep {
//------------------------------------------------------------------------------
// Send
//------------------------------------------------------------------------------
opentracing::expected<void> InMemorySyncTransporter::Send(
    const google::protobuf::Message& request,
    google::protobuf::Message& response) {
  if (should_throw_) {
    throw std::runtime_error{"should_throw_ == true"};
  }
  const collector::ReportRequest& report =
      dynamic_cast<const collector::ReportRequest&>(request);
  reports_.push_back(report);

  spans_.reserve(spans_.size() + report.spans_size());
  for (auto& span : report.spans()) {
    spans_.push_back(span);
  }
  response.CopyFrom(*Transporter::MakeCollectorResponse());
  if (should_disable_) {
    collector::Command command;
    command.set_disable(true);
    auto& report_response = dynamic_cast<collector::ReportResponse&>(response);
    *report_response.add_commands() = command;
  }
  return {};
}
}  // namespace lightstep
