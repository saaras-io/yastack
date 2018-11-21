#include <google/protobuf/util/message_differencer.h>
#include <lightstep/binary_carrier.h>
#include <lightstep/tracer.h>
#include <opentracing/noop.h>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <unordered_map>
#include "../src/lightstep_tracer_impl.h"
#include "../src/utility.h"
#include "in_memory_recorder.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>

using namespace lightstep;

//------------------------------------------------------------------------------
// TextMapCarrier
//------------------------------------------------------------------------------
struct TextMapCarrier : opentracing::TextMapReader, opentracing::TextMapWriter {
  TextMapCarrier(std::unordered_map<std::string, std::string>& text_map_)
      : text_map(text_map_) {}

  opentracing::expected<void> Set(
      opentracing::string_view key,
      opentracing::string_view value) const override {
    text_map[key] = value;
    return {};
  }

  opentracing::expected<opentracing::string_view> LookupKey(
      opentracing::string_view key) const override {
    if (!supports_lookup) {
      return opentracing::make_unexpected(
          opentracing::lookup_key_not_supported_error);
    }
    auto iter = text_map.find(key);
    if (iter != text_map.end()) {
      return opentracing::string_view{iter->second};
    } else {
      return opentracing::make_unexpected(opentracing::key_not_found_error);
    }
  }

  opentracing::expected<void> ForeachKey(
      std::function<opentracing::expected<void>(opentracing::string_view key,
                                                opentracing::string_view value)>
          f) const override {
    ++foreach_key_call_count;
    for (const auto& key_value : text_map) {
      auto result = f(key_value.first, key_value.second);
      if (!result) return result;
    }
    return {};
  }

  bool supports_lookup = false;
  mutable int foreach_key_call_count = 0;
  std::unordered_map<std::string, std::string>& text_map;
};

//------------------------------------------------------------------------------
// HTTPHeadersCarrier
//------------------------------------------------------------------------------
struct HTTPHeadersCarrier : opentracing::HTTPHeadersReader,
                            opentracing::HTTPHeadersWriter {
  HTTPHeadersCarrier(std::unordered_map<std::string, std::string>& text_map_)
      : text_map(text_map_) {}

  opentracing::expected<void> Set(
      opentracing::string_view key,
      opentracing::string_view value) const override {
    text_map[key] = value;
    return {};
  }

  opentracing::expected<void> ForeachKey(
      std::function<opentracing::expected<void>(opentracing::string_view key,
                                                opentracing::string_view value)>
          f) const override {
    for (const auto& key_value : text_map) {
      auto result = f(key_value.first, key_value.second);
      if (!result) return result;
    }
    return {};
  }

  std::unordered_map<std::string, std::string>& text_map;
};

//------------------------------------------------------------------------------
// tests
//------------------------------------------------------------------------------
TEST_CASE("propagation") {
  auto recorder = new InMemoryRecorder();
  auto tracer = std::shared_ptr<opentracing::Tracer>{new LightStepTracerImpl{
      PropagationOptions{}, std::unique_ptr<Recorder>{recorder}}};
  std::unordered_map<std::string, std::string> text_map;
  TextMapCarrier text_map_carrier(text_map);
  HTTPHeadersCarrier http_headers_carrier(text_map);
  BinaryCarrier binary_carrier;

  SECTION("Inject, extract, inject yields the same text_map.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetBaggageItem("abc", "123");
    CHECK(tracer->Inject(span->context(), text_map_carrier));
    auto injection_map1 = text_map;
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK((span_context_maybe && span_context_maybe->get()));
    text_map.clear();
    CHECK(tracer->Inject(*span_context_maybe->get(), text_map_carrier));
    CHECK(injection_map1 == text_map);
  }

  SECTION("Inject, extract, inject yields the same BinaryCarrier.") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetBaggageItem("abc", "123");
    CHECK(
        tracer->Inject(span->context(), LightStepBinaryWriter(binary_carrier)));
    auto binary_carrier1 = binary_carrier;
    auto span_context_maybe =
        tracer->Extract(LightStepBinaryReader(&binary_carrier));
    CHECK((span_context_maybe && span_context_maybe->get()));
    CHECK(tracer->Inject(*span_context_maybe->get(),
                         LightStepBinaryWriter(binary_carrier)));
    CHECK(google::protobuf::util::MessageDifferencer::Equals(binary_carrier1,
                                                             binary_carrier));
  }

  SECTION("Inject, extract, inject yields the same binary blob.") {
    std::ostringstream oss(std::ios::binary);
    auto span = tracer->StartSpan("a");
    CHECK(span);
    span->SetBaggageItem("abc", "123");
    CHECK(tracer->Inject(span->context(), oss));
    auto blob = oss.str();
    std::istringstream iss(blob, std::ios::binary);
    auto span_context_maybe = tracer->Extract(iss);
    CHECK((span_context_maybe && span_context_maybe->get()));
    std::ostringstream oss2(std::ios::binary);
    CHECK(tracer->Inject(*span_context_maybe->get(), oss2));
    CHECK(blob == oss2.str());
  }

  SECTION(
      "Extracing a context from an empty text-map gives a null span context.") {
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK(span_context_maybe);
    CHECK(span_context_maybe->get() == nullptr);
  }

  SECTION(
      "Injecting a non-LightStep span returns invalid_span_context_error.") {
    auto noop_tracer = opentracing::MakeNoopTracer();
    CHECK(noop_tracer);
    auto span = noop_tracer->StartSpan("a");
    CHECK(span);
    auto was_successful = tracer->Inject(span->context(), text_map_carrier);
    CHECK(!was_successful);
    CHECK(was_successful.error() == opentracing::invalid_span_context_error);
  }

  SECTION(
      "Extracting a span context with missing fields returns "
      "span_context_corrupted_error") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    CHECK(tracer->Inject(span->context(), text_map_carrier));

    // Remove a field to get an invalid span context.
    text_map.erase(std::begin(text_map));
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK(!span_context_maybe);
    CHECK(span_context_maybe.error() ==
          opentracing::span_context_corrupted_error);
  }

  SECTION("Extract is insensitive to changes in case for http header fields") {
    auto span = tracer->StartSpan("a");
    CHECK(span);
    CHECK(tracer->Inject(span->context(), http_headers_carrier));

    // Change the case of one of the fields.
    auto key_value = *std::begin(text_map);
    text_map.erase(std::begin(text_map));
    auto key = key_value.first;
    key[0] = key[0] == std::toupper(key[0])
                 ? static_cast<char>(std::tolower(key[0]))
                 : static_cast<char>(std::toupper(key[0]));
    text_map[key] = key_value.second;
    CHECK(tracer->Extract(http_headers_carrier));
  }

  SECTION("Extract/Inject fail if a stream has failure bits set.") {
    std::ostringstream oss(std::ios::binary);
    oss.setstate(std::ios_base::failbit);
    auto span = tracer->StartSpan("a");
    CHECK(span);
    CHECK(!tracer->Inject(span->context(), oss));
    oss.clear();
    CHECK(tracer->Inject(span->context(), oss));
    auto blob = oss.str();
    std::istringstream iss(blob, std::ios::binary);
    iss.setstate(std::ios_base::failbit);
    CHECK(!tracer->Extract(iss));
  }

  SECTION(
      "Extracting a span from an invalid binary blob returns "
      "span_context_corrupted_error.") {
    std::string invalid_context = "abc123";
    std::istringstream iss{invalid_context, std::ios::binary};
    auto span_context_maybe = tracer->Extract(iss);
    CHECK(!span_context_maybe);
    CHECK(span_context_maybe.error() ==
          opentracing::span_context_corrupted_error);
  }

  SECTION("Calling Extract on an empty stream yields a nullptr.") {
    std::string blob;
    std::istringstream iss(blob, std::ios::binary);
    auto span_context_maybe = tracer->Extract(iss);
    CHECK(span_context_maybe);
    CHECK(span_context_maybe->get() == nullptr);
  }
}

TEST_CASE("propagation - single key") {
  auto recorder = new InMemoryRecorder();
  PropagationOptions propagation_options;
  propagation_options.use_single_key = true;
  auto tracer = std::shared_ptr<opentracing::Tracer>{new LightStepTracerImpl{
      propagation_options, std::unique_ptr<Recorder>{recorder}}};
  std::unordered_map<std::string, std::string> text_map;
  TextMapCarrier text_map_carrier(text_map);
  auto span = tracer->StartSpan("a");
  CHECK(span);
  span->SetBaggageItem("abc", "123");
  CHECK(tracer->Inject(span->context(), text_map_carrier));
  CHECK(text_map.size() == 1);

  SECTION("Inject, extract, inject yields the same text_map.") {
    auto injection_map1 = text_map;
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK((span_context_maybe && span_context_maybe->get()));
    text_map.clear();
    CHECK(tracer->Inject(*span_context_maybe->get(), text_map_carrier));
    CHECK(injection_map1 == text_map);
  }

  SECTION("If a carrier supports LookupKey, then ForeachKey won't be called") {
    text_map_carrier.supports_lookup = true;
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK((span_context_maybe && span_context_maybe->get()));
    CHECK(text_map_carrier.foreach_key_call_count == 0);
  }

  SECTION(
      "When LookupKey is used, a nullptr is returned if there is no "
      "span_context") {
    text_map.clear();
    text_map_carrier.supports_lookup = true;
    auto span_context_maybe = tracer->Extract(text_map_carrier);
    CHECK((span_context_maybe && span_context_maybe->get() == nullptr));
    CHECK(text_map_carrier.foreach_key_call_count == 0);
  }

  SECTION("Verify only valid base64 characters are used.") {
    // Follows the guidelines given in RFC-4648 on what characters are
    // permissible. See
    //    http://www.rfc-editor.org/rfc/rfc4648.txt
    auto iter = text_map.begin();
    CHECK(iter != text_map.end());
    auto value = iter->second;
    auto is_base64_char = [](char c) {
      return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') ||
             ('0' <= c && c <= '9') || c == '+' || c == '/' || c == '=';
    };
    CHECK(std::all_of(value.begin(), value.end(), is_base64_char));
    CHECK(value.size() % 4 == 0);
  }
}
