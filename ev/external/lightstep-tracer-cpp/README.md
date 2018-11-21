# lightstep-tracer-cpp
[![MIT license](http://img.shields.io/badge/license-MIT-blue.svg)](http://opensource.org/licenses/MIT)

The LightStep distributed tracing library for C++.

## Installation

```
$ mkdir .build
$ cd .build
$ cmake ..
$ make
$ sudo make install
```

## Getting started

To initialize the LightStep library in particular, either retain a reference to
the LightStep `opentracing::Tracer` implementation and/or set the global
`Tracer` like so:

```cpp
#include <lightstep/tracer.h>
using namespace lightstep;

int main() {
  // Initialize the LightStep Tracer; see lightstep::LightStepTracerOptions for
  // tuning, etc.
  LightStepTracerOptions options;
  options.access_token = "YourAccessToken";
  auto tracer = MakeLightStepTracer(options);

  // Optionally set the opentracing global Tracer to the above.
  opentracing::Tracer::InitGlobal(tracer);

  return 0;
}
```

For instrumentation documentation, see the [opentracing-cpp docs](https://github.com/opentracing/opentracing-cpp).
