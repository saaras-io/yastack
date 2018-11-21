#pragma once

#include <opentracing/tracer.h>
#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
int LookupSpansDropped(const collector::ReportRequest& report);

bool HasTag(const collector::Span& span, opentracing::string_view key,
            const opentracing::Value& value);

bool HasRelationship(opentracing::SpanReferenceType relationship,
                     const collector::Span& span_a,
                     const collector::Span& span_b);
}  // namespace lightstep
