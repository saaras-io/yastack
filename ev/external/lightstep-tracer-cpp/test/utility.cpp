#include "../src/utility.h"
#include <google/protobuf/util/message_differencer.h>
#include <algorithm>
#include <exception>
#include <iostream>
#include "utility.h"

namespace lightstep {
//------------------------------------------------------------------------------
// LookupSpansDropped
//------------------------------------------------------------------------------
int LookupSpansDropped(const collector::ReportRequest& report) {
  if (!report.has_internal_metrics()) {
    return 0;
  }
  auto& counts = report.internal_metrics().counts();
  auto iter = std::find_if(counts.begin(), counts.end(),
                           [](const collector::MetricsSample& sample) {
                             return sample.name() == "spans.dropped";
                           });
  if (iter == counts.end()) {
    return 0;
  }
  if (iter->value_case() != collector::MetricsSample::kIntValue) {
    std::cerr << "spans.dropped not of type int\n";
    std::terminate();
  }
  return static_cast<int>(iter->int_value());
}

//------------------------------------------------------------------------------
// HasTag
//------------------------------------------------------------------------------
bool HasTag(const collector::Span& span, opentracing::string_view key,
            const opentracing::Value& value) {
  auto key_value = ToKeyValue(key, value);
  return std::any_of(
      std::begin(span.tags()), std::end(span.tags()),
      [&](const collector::KeyValue& other) {
        return google::protobuf::util::MessageDifferencer::Equals(key_value,
                                                                  other);
      });
}

//------------------------------------------------------------------------------
// HasRelationship
//------------------------------------------------------------------------------
bool HasRelationship(opentracing::SpanReferenceType relationship,
                     const collector::Span& span_a,
                     const collector::Span& span_b) {
  collector::Reference reference;
  switch (relationship) {
    case opentracing::SpanReferenceType::ChildOfRef:
      reference.set_relationship(collector::Reference::CHILD_OF);
      break;
    case opentracing::SpanReferenceType::FollowsFromRef:
      reference.set_relationship(collector::Reference::FOLLOWS_FROM);
      break;
  }
  *reference.mutable_span_context() = span_b.span_context();
  return std::any_of(
      std::begin(span_a.references()), std::end(span_a.references()),
      [&](const collector::Reference& other) {
        return google::protobuf::util::MessageDifferencer::Equals(reference,
                                                                  other);
      });
}
}  // namespace lightstep
