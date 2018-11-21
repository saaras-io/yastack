#include "../src/manual_recorder.h"
#include <lightstep/tracer.h>
#include <atomic>
#include "../src/lightstep_tracer_impl.h"
#include "counting_metrics_observer.h"
#include "in_memory_async_transporter.h"
#include "testing_condition_variable_wrapper.h"
#include "utility.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>

using namespace lightstep;
using namespace opentracing;

TEST_CASE("manual_recorder") {
  Logger logger{};
  auto metrics_observer = new CountingMetricsObserver{};
  LightStepTracerOptions options;
  size_t max_buffered_spans{5};
  options.max_buffered_spans =
      std::function<size_t()>{[&] { return max_buffered_spans; }};
  options.metrics_observer.reset(metrics_observer);
  auto in_memory_transporter = new InMemoryAsyncTransporter{};
  auto recorder = new ManualRecorder{
      logger, std::move(options),
      std::unique_ptr<AsyncTransporter>{in_memory_transporter}};
  auto tracer = std::shared_ptr<LightStepTracer>{new LightStepTracerImpl{
      PropagationOptions{}, std::unique_ptr<Recorder>{recorder}}};
  CHECK(tracer);

  SECTION("Buffered spans get transported after Flush is manually called.") {
    auto span = tracer->StartSpan("abc");
    CHECK(span);
    span->Finish();
    CHECK(in_memory_transporter->reports().size() == 0);
    CHECK(tracer->Flush());
    in_memory_transporter->Write();
    CHECK(in_memory_transporter->reports().size() == 1);
  }

  SECTION("Flush fails if a report is already being sent.") {
    auto span1 = tracer->StartSpan("abc");
    CHECK(span1);
    span1->Finish();
    CHECK(tracer->Flush());
    auto span2 = tracer->StartSpan("xyz");
    CHECK(span2);
    span2->Finish();
    CHECK(!tracer->Flush());
  }

  SECTION(
      "If the tranporter fails, it's spans are reported as dropped in the "
      "following report.") {
    logger.set_level(LogLevel::off);
    auto span1 = tracer->StartSpan("abc");
    CHECK(span1);
    span1->Finish();
    CHECK(tracer->Flush());
    in_memory_transporter->Fail(
        std::make_error_code(std::errc::network_unreachable));

    auto span2 = tracer->StartSpan("xyz");
    CHECK(span2);
    span2->Finish();
    CHECK(tracer->Flush());
    in_memory_transporter->Write();
    CHECK(LookupSpansDropped(in_memory_transporter->reports().at(0)) == 1);
  }

  SECTION(
      "If a collector sends back a disable command, then the tracer stops "
      "sending reports") {
    in_memory_transporter->set_should_disable(true);

    auto span = tracer->StartSpan("abc");
    span->Finish();
    CHECK(tracer->Flush());
    in_memory_transporter->Write();

    span = tracer->StartSpan("xyz");
    CHECK(span);
    span->Finish();
    CHECK(!tracer->Flush());
  }

  SECTION("Flush is called when the tracer's buffer is filled.") {
    for (size_t i = 0; i < max_buffered_spans; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }
    in_memory_transporter->Write();
    CHECK(in_memory_transporter->reports().size() == 1);
  }

  SECTION(
      "MetricsObserver::OnFlush gets called whenever the recorder is "
      "successfully flushed.") {
    auto span = tracer->StartSpan("abc");
    span->Finish();
    tracer->Flush();
    in_memory_transporter->Write();
    CHECK(metrics_observer->num_flushes == 1);
  }

  SECTION(
      "MetricsObserver::OnSpansSent gets called with the number of spans "
      "transported") {
    auto span1 = tracer->StartSpan("abc");
    span1->Finish();
    auto span2 = tracer->StartSpan("abc");
    span2->Finish();
    tracer->Flush();
    in_memory_transporter->Write();
    CHECK(metrics_observer->num_spans_sent == 2);
  }

  SECTION(
      "MetricsObserver::OnSpansDropped gets called when spans are dropped.") {
    logger.set_level(LogLevel::off);
    auto span1 = tracer->StartSpan("abc");
    span1->Finish();
    auto span2 = tracer->StartSpan("abc");
    span2->Finish();
    tracer->Flush();
    in_memory_transporter->Fail(
        std::make_error_code(std::errc::network_unreachable));
    CHECK(metrics_observer->num_spans_sent == 2);
    CHECK(metrics_observer->num_spans_dropped == 2);
  }

  SECTION(
      "If `max_buffered_spans` is dynamically changed, it takes whenever the "
      "recorder is next invoked.") {
    max_buffered_spans -= 1;
    for (size_t i = 0; i < max_buffered_spans; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }
    in_memory_transporter->Write();
    CHECK(in_memory_transporter->reports().size() == 1);
  }
}
