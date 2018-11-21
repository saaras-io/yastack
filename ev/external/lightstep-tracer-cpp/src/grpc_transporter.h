#pragma once

#include <lightstep/tracer.h>
#include "logger.h"

namespace lightstep {
std::unique_ptr<SyncTransporter> MakeGrpcTransporter(
    Logger& logger, const LightStepTracerOptions& options);
}  // namespace lightstep
