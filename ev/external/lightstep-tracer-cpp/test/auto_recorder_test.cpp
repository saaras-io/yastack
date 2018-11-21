#include "../src/auto_recorder.h"
#include <lightstep/tracer.h>
#include <atomic>
#include "../src/lightstep_tracer_impl.h"
#include "counting_metrics_observer.h"
#include "in_memory_sync_transporter.h"
#include "testing_condition_variable_wrapper.h"
#include "utility.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>

using namespace lightstep;
using namespace opentracing;

TEST_CASE("auto_recorder") {
  Logger logger{};
  auto metrics_observer = new CountingMetricsObserver{};
  LightStepTracerOptions options;
  const auto reporting_period = std::chrono::milliseconds{2};
  std::atomic<size_t> max_buffered_spans{5};
  options.reporting_period = reporting_period;
  options.max_buffered_spans =
      std::function<size_t()>{[&] { return size_t{max_buffered_spans}; }};
  options.metrics_observer.reset(metrics_observer);
  auto in_memory_transporter = new InMemorySyncTransporter{};
  auto condition_variable = new TestingConditionVariableWrapper{};
  auto recorder = new AutoRecorder{
      logger, std::move(options),
      std::unique_ptr<SyncTransporter>{in_memory_transporter},
      std::unique_ptr<ConditionVariableWrapper>{condition_variable}};
  auto tracer = std::shared_ptr<opentracing::Tracer>{new LightStepTracerImpl{
      PropagationOptions{}, std::unique_ptr<Recorder>{recorder}}};
  CHECK(tracer);

  // Ensure that the writer thread is waiting.
  condition_variable->WaitTillNextEvent();

  SECTION(
      "The writer thread waits until `now() + reporting_period` to "
      "send a report") {
    auto now = condition_variable->Now();
    condition_variable->WaitTillNextEvent();
    auto event = condition_variable->next_event();
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::WaitEvent*>(
              event) != nullptr);
    CHECK(event->timeout() == now + reporting_period);
  }

  SECTION(
      "If the writer thread takes longer than `reporting_period` to run, then "
      "it runs again immediately upon finishing.") {
    auto now = condition_variable->Now() + 2 * reporting_period;
    condition_variable->WaitTillNextEvent();
    auto span = tracer->StartSpan("abc");
    span->Finish();
    condition_variable->set_now(now);
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();
    auto event = condition_variable->next_event();
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::WaitEvent*>(
              event) != nullptr);
    CHECK(condition_variable->Now() == now);
    CHECK(event->timeout() == now);
  }

  SECTION(
      "If the writer thread takes time between 0 and `reporting_period` to "
      "run, then it subtracts the elapse time from the next timeout") {
    auto now = condition_variable->Now() + 3 * reporting_period / 2;
    condition_variable->WaitTillNextEvent();
    auto span = tracer->StartSpan("abc");
    span->Finish();
    condition_variable->set_now(now);
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();
    auto event = condition_variable->next_event();
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::WaitEvent*>(
              event) != nullptr);
    CHECK(condition_variable->Now() == now);
    CHECK(event->timeout() == now + reporting_period / 2);
  }

  SECTION(
      "If the transporter's SendReport function throws, we drop all subsequent "
      "spans.") {
    logger.set_level(LogLevel::off);
    in_memory_transporter->set_should_throw(true);
    condition_variable->set_block_notify_all(true);

    auto span = tracer->StartSpan("abc");
    span->Finish();
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();
    condition_variable->set_block_notify_all(false);
    condition_variable->Step();
    CHECK(!recorder->is_writer_running());
  }

  SECTION(
      "If a collector sends back a disable command, then the tracer stops "
      "sending reports") {
    in_memory_transporter->set_should_disable(true);
    condition_variable->set_block_notify_all(true);

    auto span = tracer->StartSpan("abc");
    span->Finish();
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();
    condition_variable->set_block_notify_all(false);
    condition_variable->Step();
    CHECK(!recorder->is_writer_running());
  }

  SECTION(
      "Dropped spans counts get sent in the next ReportRequest, and cleared in "
      "the following ReportRequest.") {
    condition_variable->set_block_notify_all(true);
    for (size_t i = 0; i < max_buffered_spans + 1; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }
    // Check that a NotifyAllEvent gets added when the buffer overflows.
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::NotifyAllEvent*>(
              condition_variable->next_event()) != nullptr);

    // Wait until the first report gets sent.
    condition_variable->Step();
    condition_variable->Step();
    condition_variable->set_block_notify_all(false);

    auto span = tracer->StartSpan("xyz");
    CHECK(span);
    span->Finish();
    // Ensure that the second report gets sent.
    condition_variable->Step();
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();

    auto reports = in_memory_transporter->reports();
    CHECK(reports.size() == 2);
    CHECK(LookupSpansDropped(reports.at(0)) == 1);
    CHECK(reports.at(0).spans_size() == max_buffered_spans);
    CHECK(LookupSpansDropped(reports.at(1)) == 0);
    CHECK(reports.at(1).spans_size() == 1);
  }

  SECTION(
      "MetricsObserver::OnFlush gets called whenever the recorder is "
      "successfully flushed.") {
    auto span = tracer->StartSpan("abc");
    span->Finish();
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();
    CHECK(metrics_observer->num_flushes == 1);
  }

  SECTION(
      "MetricsObserver::OnSpansSent gets called with the number of spans "
      "transported") {
    auto span1 = tracer->StartSpan("abc");
    span1->Finish();
    auto span2 = tracer->StartSpan("abc");
    span2->Finish();
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();
    CHECK(metrics_observer->num_spans_sent == 2);
  }

  SECTION(
      "MetricsObserver::OnSpansDropped gets called when spans are dropped.") {
    condition_variable->set_block_notify_all(true);
    for (size_t i = 0; i < max_buffered_spans + 1; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }
    condition_variable->set_block_notify_all(false);
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();
    CHECK(metrics_observer->num_spans_sent == max_buffered_spans);
    CHECK(metrics_observer->num_spans_dropped == 1);
  }

  SECTION(
      "If `max_buffered_spans` is changed dynamically, it will take effect "
      "after the next Flush") {
    condition_variable->set_block_notify_all(true);
    size_t max_buffered_spans_old = max_buffered_spans;
    size_t max_buffered_spans_new = max_buffered_spans_old - 1;
    max_buffered_spans = max_buffered_spans_new;
    for (size_t i = 0; i < max_buffered_spans_new; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }

    // Check that no NotifyAllEvent was added.
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::NotifyAllEvent*>(
              condition_variable->next_event()) == nullptr);
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();

    for (size_t i = 0; i < max_buffered_spans_new; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }

    // Check that a NotifyAllEvent was added.
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::NotifyAllEvent*>(
              condition_variable->next_event()) != nullptr);
    condition_variable->set_block_notify_all(false);
    condition_variable->Step();
    condition_variable->WaitTillNextEvent();
  }
}
