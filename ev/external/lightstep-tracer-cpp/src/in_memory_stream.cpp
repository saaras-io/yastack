#include "in_memory_stream.h"

namespace lightstep {
//------------------------------------------------------------------------------
// in_memory_buffer constructor
//------------------------------------------------------------------------------
in_memory_buffer::in_memory_buffer(const char* data, size_t size) {
  // Data isn't modified so this is safe.
  auto non_const_data = const_cast<char*>(data);
  setg(non_const_data, non_const_data, non_const_data + size);
}

//------------------------------------------------------------------------------
// in_memory_stream constructor
//------------------------------------------------------------------------------
in_memory_stream::in_memory_stream(const char* data, size_t size)
    : in_memory_buffer{data, size},
      std::istream{static_cast<std::streambuf*>(this)} {}
}  // namespace lightstep
