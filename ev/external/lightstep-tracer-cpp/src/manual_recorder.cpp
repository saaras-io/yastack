#include "manual_recorder.h"
#include "utility.h"

namespace lightstep {
//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
ManualRecorder::ManualRecorder(Logger& logger, LightStepTracerOptions options,
                               std::unique_ptr<AsyncTransporter>&& transporter)
    : logger_{logger},
      options_{std::move(options)},
      builder_{options_.access_token, options_.tags},
      transporter_{std::move(transporter)} {
  // If no MetricsObserver was provided, use a default one that does nothing.
  if (options_.metrics_observer == nullptr) {
    options_.metrics_observer.reset(new MetricsObserver{});
  }
}

//------------------------------------------------------------------------------
// RecordSpan
//------------------------------------------------------------------------------
void ManualRecorder::RecordSpan(collector::Span&& span) noexcept try {
  if (disabled_) {
    dropped_spans_++;
    options_.metrics_observer->OnSpansDropped(1);
    return;
  }

  auto max_buffered_spans = options_.max_buffered_spans.value();
  if (builder_.num_pending_spans() >= max_buffered_spans) {
    // If there's no report in flight, flush the recoder. We can only get
    // here if max_buffered_spans was dynamically decreased.
    //
    // Otherwise, drop the span.
    if (!IsReportInProgress()) {
      FlushOne();
    } else {
      dropped_spans_++;
      options_.metrics_observer->OnSpansDropped(1);
      return;
    }
  }
  builder_.AddSpan(std::move(span));
  if (builder_.num_pending_spans() >= max_buffered_spans) {
    FlushOne();
  }
} catch (const std::exception& e) {
  logger_.Error("Failed to record span: ", e.what());
}

//------------------------------------------------------------------------------
// IsReportInProgress
//------------------------------------------------------------------------------
bool ManualRecorder::IsReportInProgress() const noexcept {
  return encoding_seqno_ > 1 + flushed_seqno_;
}

//------------------------------------------------------------------------------
// FlushOne
//------------------------------------------------------------------------------
bool ManualRecorder::FlushOne() noexcept try {
  options_.metrics_observer->OnFlush();

  // If a report is currently in flight, do nothing; and if there are any
  // pending spans, then the flush is considered to have failed.
  if (IsReportInProgress()) {
    return builder_.num_pending_spans() == 0;
  }

  saved_pending_spans_ = builder_.num_pending_spans();
  if (saved_pending_spans_ == 0) {
    return true;
  }
  options_.metrics_observer->OnSpansSent(
      static_cast<int>(saved_pending_spans_));
  saved_dropped_spans_ = dropped_spans_;
  builder_.set_pending_client_dropped_spans(dropped_spans_);
  dropped_spans_ = 0;
  std::swap(builder_.pending(), active_request_);
  ++encoding_seqno_;
  transporter_->Send(active_request_, active_response_, *this);
  return true;
} catch (const std::exception& e) {
  logger_.Error("Failed to Flush: ", e.what());
  options_.metrics_observer->OnSpansDropped(saved_pending_spans_);
  dropped_spans_ += saved_pending_spans_;
  active_request_.Clear();
  return false;
}

//------------------------------------------------------------------------------
// FlushWithTimeout
//------------------------------------------------------------------------------
bool ManualRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration /*timeout*/) noexcept {
  if (disabled_) {
    return false;
  }
  return FlushOne();
}

//------------------------------------------------------------------------------
// OnSuccess
//------------------------------------------------------------------------------
void ManualRecorder::OnSuccess() noexcept {
  ++flushed_seqno_;
  active_request_.Clear();
  LogReportResponse(logger_, options_.verbose, active_response_);
  for (auto& command : active_response_.commands()) {
    if (command.disable()) {
      logger_.Warn("Tracer disabled by collector");
      disabled_ = true;
    }
  }
}

//------------------------------------------------------------------------------
// OnFailure
//------------------------------------------------------------------------------
void ManualRecorder::OnFailure(std::error_code error) noexcept {
  ++flushed_seqno_;
  active_request_.Clear();
  options_.metrics_observer->OnSpansDropped(
      static_cast<int>(saved_pending_spans_));
  dropped_spans_ += saved_dropped_spans_ + saved_pending_spans_;
  logger_.Error("Failed to send report: ", error.message());
}
}  // namespace lightstep
