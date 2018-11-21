#include <lightstep/tracer.h>
#include <cassert>
#include <cstdlib>  // for std::getenv
#include <iostream>
#include <unordered_map>
#include "text_map_carrier.h"
using namespace lightstep;
using namespace opentracing;

int main() {
  LightStepTracerOptions options;
  options.verbose = true;
  options.component_name = "Tutorial";

  // Optionally, by setting `options.logger_sink`, you can customize where logs
  // from the LightStep tracer are recorded.
  options.logger_sink = [](lightstep::LogLevel /*level*/,
                           opentracing::string_view message) {
    std::cout << "Tracer's Log: " << message << '\n';
  };

  if (const char* access_token = std::getenv("LIGHTSTEP_ACCESS_TOKEN")) {
    options.access_token = access_token;
  } else {
    std::cerr << "You must set the environmental variable "
                 "`LIGHTSTEP_ACCESS_TOKEN` to your access token!\n";
    return -1;
  }
  auto tracer = MakeLightStepTracer(std::move(options));
  assert(tracer);

  auto parent_span = tracer->StartSpan("parent");
  assert(parent_span);

  // Create a child span.
  {
    auto child_span =
        tracer->StartSpan("childA", {ChildOf(&parent_span->context())});
    assert(child_span);

    // Set a simple tag.
    child_span->SetTag("simple tag", 123);

    // Set a complex tag.
    child_span->SetTag("complex tag",
                       Values{123, Dictionary{{"abc", 123}, {"xyz", 4.0}}});

    // Log simple values.
    child_span->Log({{"event", "simple log"}, {"abc", 123}});

    // Log complex values.
    child_span->Log({{"event", "complex log"},
                     {"data", Dictionary{{"a", 1}, {"b", Values{1, 2}}}}});

    child_span->Finish();
  }

  // Create a follows from span.
  {
    auto child_span =
        tracer->StartSpan("childB", {FollowsFrom(&parent_span->context())});

    // child_span's destructor will finish the span if not done so explicitly.
  }

  // Use custom timestamps.
  {
    auto t1 = SystemClock::now();
    auto t2 = SteadyClock::now();
    auto span = tracer->StartSpan(
        "useCustomTimestamps",
        {ChildOf(&parent_span->context()), StartTimestamp(t1)});
    assert(span);
    span->Finish({FinishTimestamp(t2)});
  }

  // Extract and Inject a span context.
  {
    std::unordered_map<std::string, std::string> text_map;
    TextMapCarrier carrier(text_map);
    auto err = tracer->Inject(parent_span->context(), carrier);
    assert(err);
    auto span_context_maybe = tracer->Extract(carrier);
    assert(span_context_maybe);
    auto span = tracer->StartSpan("propagationSpan",
                                  {ChildOf(span_context_maybe->get())});
  }

  // You get an error when trying to extract a corrupt span.
  {
    std::unordered_map<std::string, std::string> text_map = {
        {"ot-tracer-traceid", "123"}};
    TextMapCarrier carrier(text_map);
    auto err = tracer->Extract(carrier);
    assert(!err);
    assert(err.error() == span_context_corrupted_error);
    // How to get a readable message from the error.
    std::cout << "Example error message: \"" << err.error().message() << "\"\n";
  }

  parent_span->Finish();
  tracer->Close();

  return 0;
}
