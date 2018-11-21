#pragma once

#include <lightstep/tracer.h>
#include <lightstep/transporter.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include "condition_variable_wrapper.h"
#include "lightstep-tracer-common/collector.pb.h"
#include "logger.h"
#include "recorder.h"
#include "report_builder.h"

namespace lightstep {
// AutoRecorder buffers spans finished by a tracer and sends them over to
// the provided SyncTransporter. It uses an internal thread to regularly send
// the reports according to the rate specified by LightStepTracerOptions.
class AutoRecorder : public Recorder {
 public:
  AutoRecorder(Logger& logger, LightStepTracerOptions&& options,
               std::unique_ptr<SyncTransporter>&& transporter);

  AutoRecorder(Logger& logger, LightStepTracerOptions&& options,
               std::unique_ptr<SyncTransporter>&& transporter,
               std::unique_ptr<ConditionVariableWrapper>&& write_cond);

  AutoRecorder(const AutoRecorder&) = delete;
  AutoRecorder(AutoRecorder&&) = delete;
  AutoRecorder& operator=(const AutoRecorder&) = delete;
  AutoRecorder& operator=(AutoRecorder&&) = delete;

  ~AutoRecorder() override;

  void RecordSpan(collector::Span&& span) noexcept override;

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

  // used for testing only.
  bool is_writer_running() const {
    std::lock_guard<std::mutex> lock_guard{write_mutex_};
    return !write_exit_;
  }

 private:
  void Write() noexcept;
  bool WriteReport(const collector::ReportRequest& report);
  void FlushOne();

  // Forces the writer thread to exit immediately.
  void MakeWriterExit();

  // Waits until either the timeout or the writer thread is forced to
  // exit.  Returns true if it should continue writing, false if it
  // should exit.
  bool WaitForNextWrite(const std::chrono::steady_clock::time_point& next);

  Logger& logger_;
  LightStepTracerOptions options_;

  // Writer state.
  mutable std::mutex write_mutex_;
  bool write_exit_ = false;
  std::thread writer_;

  // Buffer state (protected by write_mutex_).
  ReportBuilder builder_;
  collector::ReportRequest inflight_;
  size_t max_buffered_spans_snapshot_;
  size_t flushed_seqno_ = 0;
  size_t encoding_seqno_ = 1;
  size_t dropped_spans_ = 0;

  // SyncTransporter through which to send span reports.
  std::unique_ptr<SyncTransporter> transporter_;

  std::unique_ptr<ConditionVariableWrapper> write_cond_;
};
}  // namespace lightstep
