#pragma once

#include <google/protobuf/message.h>
#include <opentracing/string_view.h>
#include <opentracing/util.h>

namespace lightstep {
// Transporter is the abstract base class for SyncTransporter and
// AsyncTransporter.
class Transporter {
 public:
  virtual ~Transporter() = default;

  // Generates a valid artificial collector response.
  //
  // Can be used by testing code to to simulate the interaction with a LightStep
  // collector.
  static std::unique_ptr<google::protobuf::Message> MakeCollectorResponse();
};

// SyncTransporter customizes how synchronous tracing reports are sent.
class SyncTransporter : public Transporter {
 public:
  // Synchronously sends `request` to a collector and sets `response` to the
  // collector's response.
  virtual opentracing::expected<void> Send(
      const google::protobuf::Message& request,
      google::protobuf::Message& response) = 0;
};

// AsyncTransporter customizes how asynchronous tracing reports are sent.
class AsyncTransporter : public Transporter {
 public:
  // Callback interface used by Send.
  class Callback {
   public:
    virtual ~Callback() = default;

    virtual void OnSuccess() noexcept = 0;

    virtual void OnFailure(std::error_code error) noexcept = 0;
  };

  // Asynchronously sends `request` to a collector.
  //
  // On success, `response` is set to the collector's response and
  // `callback.OnSuccess()` should be called.
  //
  // On failure, `callback.OnFailure(error)` should be called.
  virtual void Send(const google::protobuf::Message& request,
                    google::protobuf::Message& response,
                    Callback& callback) = 0;
};
}  // namespace lightstep
