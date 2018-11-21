// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.#pragma once

#pragma once

#include <functional>
#include "jwt_verify_lib/jwt.h"

namespace google {
namespace jwt_verify {

/**
 * This function fuzz the signature in two loops
 */
void fuzzJwtSignature(const Jwt& jwt,
                      std::function<void(const Jwt& jwt)> test_fn) {
  // alter 1 bit
  for (size_t b = 0; b < jwt.signature_.size(); ++b) {
    for (int bit = 0; bit < 8; ++bit) {
      Jwt fuzz_jwt(jwt);
      unsigned char bb = fuzz_jwt.signature_[b];
      bb ^= (unsigned char)(1 << bit);
      fuzz_jwt.signature_[b] = (char)bb;
      test_fn(fuzz_jwt);
    }
  }

  // truncate bytes
  for (size_t pos = 0; pos < jwt.signature_.size(); ++pos) {
    for (size_t count = 1; count < jwt.signature_.size() - pos; ++count) {
      Jwt fuzz_jwt(jwt);
      fuzz_jwt.signature_ = jwt.signature_.substr(pos, count);
      test_fn(fuzz_jwt);
    }
  }
}

}  // namespace jwt_verify
}  // namespace google
