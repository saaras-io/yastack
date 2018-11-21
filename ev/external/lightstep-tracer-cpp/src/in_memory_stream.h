#pragma once

#include <istream>
#include <streambuf>

namespace lightstep {
// Classes to allow construction of an istream from constant memory without
// copying. See https://stackoverflow.com/a/13059195/4447365.
class in_memory_buffer : public std::streambuf {
 public:
  in_memory_buffer(const char* data, size_t size);
};

class in_memory_stream : virtual public in_memory_buffer, public std::istream {
 public:
  in_memory_stream(const char* data, size_t size);
};
}  // namespace lightstep
