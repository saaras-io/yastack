#include "grpc_transporter.h"
#include <lightstep/config.h>

#ifdef LIGHTSTEP_USE_GRPC
#include <grpc++/create_channel.h>
#include <chrono>
#include <sstream>
#include "lightstep-tracer-common/collector.grpc.pb.h"
#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
//------------------------------------------------------------------------------
// HostPortOf
//------------------------------------------------------------------------------
static std::string HostPortOf(const LightStepTracerOptions& options) {
  std::ostringstream os;
  os << options.collector_host << ":" << options.collector_port;
  return os.str();
}

//------------------------------------------------------------------------------
// MakeErrorCode
//------------------------------------------------------------------------------
// Try to map grpc::StatusCode to stanard POSIX error codes. Follows
// descriptions from
//    https://github.com/grpc/grpc/blob/master/doc/statuscodes.md
static std::error_code MakeErrorCode(grpc::StatusCode status_code) {
  switch (status_code) {
    case grpc::CANCELLED:
      return std::make_error_code(std::errc::operation_canceled);
    case grpc::DEADLINE_EXCEEDED:
      return std::make_error_code(std::errc::timed_out);
    case grpc::RESOURCE_EXHAUSTED:
      return std::make_error_code(std::errc::resource_unavailable_try_again);
    case grpc::UNIMPLEMENTED:
      return std::make_error_code(std::errc::not_supported);
    case grpc::UNAVAILABLE:
      return std::make_error_code(std::errc::network_down);
    default:
      return std::error_code{};
  }
}

//------------------------------------------------------------------------------
// GrpcTransporter
//------------------------------------------------------------------------------
namespace {
// GrpcTransporter sends ReportRequests to the specified host via gRPC.
class GrpcTransporter : public SyncTransporter {
 public:
  GrpcTransporter(Logger& logger, const LightStepTracerOptions& options)
      : logger_{logger},
        client_{grpc::CreateChannel(
            HostPortOf(options),
            options.collector_plaintext
                ? grpc::InsecureChannelCredentials()
                : grpc::SslCredentials(grpc::SslCredentialsOptions()))},
        report_timeout_{options.report_timeout} {}

  opentracing::expected<void> Send(
      const google::protobuf::Message& request,
      google::protobuf::Message& response) override {
    grpc::ClientContext context;
    collector::ReportResponse resp;
    context.set_fail_fast(true);
    context.set_deadline(std::chrono::system_clock::now() + report_timeout_);
    auto status = client_.Report(
        &context, dynamic_cast<const collector::ReportRequest&>(request),
        dynamic_cast<collector::ReportResponse*>(&response));
    if (!status.ok()) {
      logger_.Error("Report RPC failed: ", status.error_message());
      return opentracing::make_unexpected(MakeErrorCode(status.error_code()));
    }
    return {};
  }

 private:
  Logger& logger_;
  // Collector service stub.
  collector::CollectorService::Stub client_;
  std::chrono::system_clock::duration report_timeout_;
};
}  // anonymous namespace

//------------------------------------------------------------------------------
// MakeGrpcTransporter
//------------------------------------------------------------------------------
std::unique_ptr<SyncTransporter> MakeGrpcTransporter(
    Logger& logger, const LightStepTracerOptions& options) {
  return std::unique_ptr<SyncTransporter>{new GrpcTransporter{logger, options}};
}
}  // namespace lightstep
#else
#include <stdexcept>
namespace lightstep {
std::unique_ptr<SyncTransporter> MakeGrpcTransporter(
    Logger& /*logger*/, const LightStepTracerOptions& /*options*/) {
  throw std::runtime_error{
      "LightStep was not built with gRPC support, so a transporter must be "
      "supplied."};
}
}  // namespace lightstep
#endif
