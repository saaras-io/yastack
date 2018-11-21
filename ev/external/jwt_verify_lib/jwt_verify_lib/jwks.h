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

#include <string>
#include <vector>

#include "jwt_verify_lib/status.h"

#include "openssl/ec.h"
#include "openssl/evp.h"

namespace google {
namespace jwt_verify {

/**
 *  Class to parse and a hold JSON Web Key Set.
 *
 *  Usage example:
 *    JwksPtr keys = Jwks::createFrom(jwks_string, type);
 *    if (keys->getStatus() == Status::Ok) { ... }
 */
class Jwks : public WithStatus {
 public:
  // Format of public key.
  enum Type { PEM, JWKS };

  // Create from string
  static std::unique_ptr<Jwks> createFrom(const std::string& pkey, Type type);

  // Struct for JSON Web Key
  struct Pubkey {
    bssl::UniquePtr<EVP_PKEY> evp_pkey_;
    bssl::UniquePtr<EC_KEY> ec_key_;
    std::string kid_;
    std::string kty_;
    std::string alg_;
    bool alg_specified_ = false;
    bool kid_specified_ = false;
    bool pem_format_ = false;
  };
  typedef std::unique_ptr<Pubkey> PubkeyPtr;

  // Access to list of Jwks
  const std::vector<PubkeyPtr>& keys() const { return keys_; }

 private:
  // Create Pem
  void createFromPemCore(const std::string& pkey_pem);
  // Create Jwks
  void createFromJwksCore(const std::string& pkey_jwks);

  // List of Jwks
  std::vector<PubkeyPtr> keys_;
};

typedef std::unique_ptr<Jwks> JwksPtr;

}  // namespace jwt_verify
}  // namespace google
