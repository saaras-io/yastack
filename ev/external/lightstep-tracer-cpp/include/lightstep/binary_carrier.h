#pragma once

#include <opentracing/propagation.h>
#include "lightstep-tracer-common/lightstep_carrier.pb.h"

namespace lightstep {
class LightStepBinaryReader : public opentracing::CustomCarrierReader {
 public:
  LightStepBinaryReader(const BinaryCarrier* carrier) noexcept
      : carrier_{carrier} {}

  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      const opentracing::Tracer& tracer) const override;

 private:
  const BinaryCarrier* carrier_;
};

class LightStepBinaryWriter : public opentracing::CustomCarrierWriter {
 public:
  LightStepBinaryWriter(BinaryCarrier& carrier) noexcept : carrier_{carrier} {}

  opentracing::expected<void> Inject(
      const opentracing::Tracer& tracer,
      const opentracing::SpanContext& span_context) const override;

 private:
  BinaryCarrier& carrier_;
};
}  // namespace lightstep
