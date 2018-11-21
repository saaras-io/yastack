#pragma once

#include <lightstep/transporter.h>
#include "logger.h"
#include "recorder.h"
#include "report_builder.h"

namespace lightstep {
// ManualRecorder buffers spans finished by a tracer and sends them over to
// the provided AsyncTransporter when FlushWithTimeout is called.
class ManualRecorder : public Recorder, private AsyncTransporter::Callback {
 public:
  ManualRecorder(Logger& logger, LightStepTracerOptions options,
                 std::unique_ptr<AsyncTransporter>&& transporter);

  void RecordSpan(collector::Span&& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  bool IsReportInProgress() const noexcept;

  bool FlushOne() noexcept;

  void OnSuccess() noexcept override;
  void OnFailure(std::error_code error) noexcept override;

  Logger& logger_;
  LightStepTracerOptions options_;

  bool disabled_ = false;

  // Buffer state
  ReportBuilder builder_;
  collector::ReportRequest active_request_;
  collector::ReportResponse active_response_;
  size_t saved_dropped_spans_ = 0;
  size_t saved_pending_spans_ = 0;
  size_t flushed_seqno_ = 0;
  size_t encoding_seqno_ = 1;
  size_t dropped_spans_ = 0;

  // AsyncTransporter through which to send span reports.
  std::unique_ptr<AsyncTransporter> transporter_;
};
}  // namespace lightstep
