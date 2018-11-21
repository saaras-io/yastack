#include "auto_recorder.h"
#include <exception>
#include "utility.h"

namespace lightstep {
//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
AutoRecorder::AutoRecorder(Logger& logger, LightStepTracerOptions&& options,
                           std::unique_ptr<SyncTransporter>&& transporter)
    : AutoRecorder{logger, std::move(options), std::move(transporter),
                   std::unique_ptr<ConditionVariableWrapper>{
                       new StandardConditionVariableWrapper{}}} {}

AutoRecorder::AutoRecorder(
    Logger& logger, LightStepTracerOptions&& options,
    std::unique_ptr<SyncTransporter>&& transporter,
    std::unique_ptr<ConditionVariableWrapper>&& write_cond)
    : logger_{logger},
      options_{std::move(options)},
      builder_{options_.access_token, options_.tags},
      transporter_{std::move(transporter)},
      write_cond_{std::move(write_cond)} {
  // If no MetricsObserver was provided, use a default one that does nothing.
  if (options_.metrics_observer == nullptr) {
    options_.metrics_observer.reset(new MetricsObserver{});
  }
  max_buffered_spans_snapshot_ = options_.max_buffered_spans.value();
  writer_ = std::thread(&AutoRecorder::Write, this);
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
AutoRecorder::~AutoRecorder() {
  MakeWriterExit();
  writer_.join();
}

//------------------------------------------------------------------------------
// RecordSpan
//------------------------------------------------------------------------------
void AutoRecorder::RecordSpan(collector::Span&& span) noexcept try {
  std::lock_guard<std::mutex> lock_guard{write_mutex_};
  if (builder_.num_pending_spans() >= max_buffered_spans_snapshot_ ||
      write_exit_) {
    dropped_spans_++;
    options_.metrics_observer->OnSpansDropped(1);
    return;
  }
  builder_.AddSpan(std::move(span));
  if (builder_.num_pending_spans() >= max_buffered_spans_snapshot_) {
    write_cond_->NotifyAll();
  }
} catch (const std::exception& e) {
  logger_.Error("Failed to record span: ", e.what());
}

//------------------------------------------------------------------------------
// FlushWithTimeout
//------------------------------------------------------------------------------
bool AutoRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept try {
  // Note: there is no effort made to speed up the flush when
  // requested, it simply waits for the regularly scheduled flush
  // operations to clear out all the presently pending data.
  std::unique_lock<std::mutex> lock{write_mutex_};

  bool has_encoded = builder_.num_pending_spans() != 0;

  if (!has_encoded && encoding_seqno_ == 1 + flushed_seqno_) {
    return true;
  }

  size_t wait_seq = encoding_seqno_ - (has_encoded ? 0 : 1);

  auto result = write_cond_->WaitFor(lock, timeout, [this, wait_seq]() {
    return write_exit_ || this->flushed_seqno_ >= wait_seq;
  });
  if (!result) {
    return false;
  }
  return this->flushed_seqno_ >= wait_seq;
} catch (const std::exception& e) {
  logger_.Error("Failed to flush recorder: ", e.what());
  return false;
}

//------------------------------------------------------------------------------
// Write
//------------------------------------------------------------------------------
void AutoRecorder::Write() noexcept try {
  auto next = write_cond_->Now() + options_.reporting_period;

  while (WaitForNextWrite(next)) {
    FlushOne();

    auto end = write_cond_->Now();
    auto elapsed = end - next;

    if (elapsed > options_.reporting_period) {
      next = end;
    } else {
      next = end + options_.reporting_period - elapsed;
    }
  }
} catch (const std::exception& e) {
  MakeWriterExit();
  logger_.Error("Fatal error shutting down writer thread: ", e.what());
}

//------------------------------------------------------------------------------
// WriteReport
//------------------------------------------------------------------------------
bool AutoRecorder::WriteReport(const collector::ReportRequest& report) {
  collector::ReportResponse response;
  auto was_successful = transporter_->Send(report, response);
  if (!was_successful) {
    return false;
  }
  LogReportResponse(logger_, options_.verbose, response);
  for (auto& command : response.commands()) {
    if (command.disable()) {
      logger_.Warn("Tracer disabled by collector");
      MakeWriterExit();
    }
  }
  return true;
}

//------------------------------------------------------------------------------
// FlushOne
//------------------------------------------------------------------------------
void AutoRecorder::FlushOne() {
  options_.metrics_observer->OnFlush();

  size_t save_dropped;
  size_t save_pending;
  {
    // Swap the pending encoder with the inflight encoder, then use
    // inflight without a lock. Assumption is that this thread is the
    // only place inflight_ is used.
    std::lock_guard<std::mutex> lock_guard{write_mutex_};
    save_pending = builder_.num_pending_spans();
    if (save_pending == 0) {
      return;
    }
    options_.metrics_observer->OnSpansSent(static_cast<int>(save_pending));
    // TODO(rnburn): Compute and set timestamp_offset_micros
    save_dropped = dropped_spans_;
    builder_.set_pending_client_dropped_spans(save_dropped);
    dropped_spans_ = 0;
    std::swap(builder_.pending(), inflight_);
    ++encoding_seqno_;
  }
  bool success = WriteReport(inflight_);
  {
    std::lock_guard<std::mutex> lock_guard{write_mutex_};
    ++flushed_seqno_;
    write_cond_->NotifyAll();
    inflight_.Clear();

    if (!success) {
      options_.metrics_observer->OnSpansDropped(static_cast<int>(save_pending));
      dropped_spans_ += save_dropped + save_pending;
    }
  }
}

//------------------------------------------------------------------------------
// MakeWriterExit
//------------------------------------------------------------------------------
void AutoRecorder::MakeWriterExit() {
  std::lock_guard<std::mutex> lock_guard{write_mutex_};
  write_exit_ = true;
  write_cond_->NotifyAll();
}

//------------------------------------------------------------------------------
// WaitForNextWrite
//------------------------------------------------------------------------------
bool AutoRecorder::WaitForNextWrite(
    const std::chrono::steady_clock::time_point& next) {
  std::unique_lock<std::mutex> lock{write_mutex_};
  max_buffered_spans_snapshot_ = options_.max_buffered_spans.value();
  write_cond_->WaitUntil(lock, next, [this]() {
    return this->write_exit_ ||
           this->builder_.num_pending_spans() >= max_buffered_spans_snapshot_;
  });
  return !write_exit_;
}
}  // namespace lightstep
