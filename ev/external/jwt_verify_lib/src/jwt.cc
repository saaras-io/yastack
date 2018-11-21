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
// limitations under the License.

#include <algorithm>

#include "absl/strings/escaping.h"
#include "absl/strings/str_split.h"
#include "jwt_verify_lib/jwt.h"

namespace google {
namespace jwt_verify {

// Maximum Jwt size to prevent JSON parser attack:
// stack overflow crash if a document contains heavily nested arrays.
// [[... repeat 100,000 times ... [[[0]]]]]]]]]]]]]]]]]]]]]]]]]]]..
const size_t kMaxJwtSize = 8096;

Jwt::Jwt(const Jwt& instance) { *this = instance; }

Jwt& Jwt::operator=(const Jwt& rhs) {
  parseFromString(rhs.jwt_);
  return *this;
}

Status Jwt::parseFromString(const std::string& jwt) {
  if (jwt.size() >= kMaxJwtSize) {
    return Status::JwtBadFormat;
  }
  // jwt must have exactly 2 dots
  if (std::count(jwt.begin(), jwt.end(), '.') != 2) {
    return Status::JwtBadFormat;
  }
  jwt_ = jwt;
  std::vector<absl::string_view> jwt_split =
      absl::StrSplit(jwt, '.', absl::SkipEmpty());
  if (jwt_split.size() != 3) {
    return Status::JwtBadFormat;
  }

  // Parse header json
  header_str_base64url_ = std::string(jwt_split[0]);
  if (!absl::WebSafeBase64Unescape(header_str_base64url_, &header_str_)) {
    return Status::JwtHeaderParseError;
  }

  if (header_json_.Parse(header_str_.c_str()).HasParseError()) {
    return Status::JwtHeaderParseError;
  }

  // Header should contain "alg" and should be a string.
  if (!header_json_.HasMember("alg") || !header_json_["alg"].IsString()) {
    return Status::JwtHeaderBadAlg;
  }
  alg_ = header_json_["alg"].GetString();

  if (alg_ != "RS256" && alg_ != "ES256") {
    return Status::JwtHeaderNotImplementedAlg;
  }

  // Header may contain "kid", should be a string if exists.
  if (header_json_.HasMember("kid")) {
    if (!header_json_["kid"].IsString()) {
      return Status::JwtHeaderBadKid;
    }
    kid_ = header_json_["kid"].GetString();
  }

  // Parse payload json
  payload_str_base64url_ = std::string(jwt_split[1]);
  if (!absl::WebSafeBase64Unescape(payload_str_base64url_, &payload_str_)) {
    return Status::JwtPayloadParseError;
  }

  if (payload_json_.Parse(payload_str_.c_str()).HasParseError()) {
    return Status::JwtPayloadParseError;
  }

  if (payload_json_.HasMember("iss")) {
    if (payload_json_["iss"].IsString()) {
      iss_ = payload_json_["iss"].GetString();
    } else {
      return Status::JwtPayloadParseError;
    }
  }
  if (payload_json_.HasMember("sub")) {
    if (payload_json_["sub"].IsString()) {
      sub_ = payload_json_["sub"].GetString();
    } else {
      return Status::JwtPayloadParseError;
    }
  }
  if (payload_json_.HasMember("iat")) {
    if (payload_json_["iat"].IsInt64()) {
      iat_ = payload_json_["iat"].GetInt64();
    } else {
      return Status::JwtPayloadParseError;
    }
  } else {
    iat_ = 0;
  }
  if (payload_json_.HasMember("nbf")) {
    if (payload_json_["nbf"].IsInt64()) {
      nbf_ = payload_json_["nbf"].GetInt64();
    } else {
      return Status::JwtPayloadParseError;
    }
  } else {
    nbf_ = 0;
  }
  if (payload_json_.HasMember("exp")) {
    if (payload_json_["exp"].IsInt64()) {
      exp_ = payload_json_["exp"].GetInt64();
    } else {
      return Status::JwtPayloadParseError;
    }
  } else {
    exp_ = 0;
  }
  if (payload_json_.HasMember("jti")) {
    if (payload_json_["jti"].IsString()) {
      jti_ = payload_json_["jti"].GetString();
    } else {
      return Status::JwtPayloadParseError;
    }
  }

  // "aud" can be either string array or string.
  // Try as string array, read it as empty array if doesn't exist.
  if (payload_json_.HasMember("aud")) {
    const auto& aud_value = payload_json_["aud"];
    if (aud_value.IsArray()) {
      for (auto it = aud_value.Begin(); it != aud_value.End(); ++it) {
        if (it->IsString()) {
          audiences_.push_back(it->GetString());
        } else {
          return Status::JwtPayloadParseError;
        }
      }
    } else if (aud_value.IsString()) {
      audiences_.push_back(aud_value.GetString());
    } else {
      return Status::JwtPayloadParseError;
    }
  }

  // Set up signature
  if (!absl::WebSafeBase64Unescape(jwt_split[2], &signature_)) {
    // Signature is a bad Base64url input.
    return Status::JwtSignatureParseError;
  }
  return Status::Ok;
}

}  // namespace jwt_verify
}  // namespace google
