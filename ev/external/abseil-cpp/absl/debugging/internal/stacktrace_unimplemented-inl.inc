#ifndef ABSL_DEBUGGING_INTERNAL_STACKTRACE_UNIMPLEMENTED_INL_H_
#define ABSL_DEBUGGING_INTERNAL_STACKTRACE_UNIMPLEMENTED_INL_H_

template <bool IS_STACK_FRAMES, bool IS_WITH_CONTEXT>
static int UnwindImpl(void** /* result */, int* /* sizes */,
                      int /* max_depth */, int /* skip_count */,
                      const void* /* ucp */, int *min_dropped_frames) {
  if (min_dropped_frames != nullptr) {
    *min_dropped_frames = 0;
  }
  return 0;
}

namespace absl {
inline namespace lts_2018_06_20 {
namespace debugging_internal {
bool StackTraceWorksForTest() {
  return false;
}
}  // namespace debugging_internal
}  // inline namespace lts_2018_06_20
}  // namespace absl

#endif  // ABSL_DEBUGGING_INTERNAL_STACKTRACE_UNIMPLEMENTED_INL_H_
