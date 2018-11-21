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

#include <assert.h>
#include <iostream>

#include "jwt_verify_lib/jwks.h"

#include "absl/strings/escaping.h"
#include "rapidjson/document.h"

#include "openssl/bn.h"
#include "openssl/ecdsa.h"
#include "openssl/evp.h"
#include "openssl/rsa.h"
#include "openssl/sha.h"

namespace google {
namespace jwt_verify {

namespace {

// A convinence inline cast function.
inline const uint8_t* castToUChar(const std::string& str) {
  return reinterpret_cast<const uint8_t*>(str.c_str());
}

/** Class to create EVP_PKEY object from string of public key, formatted in PEM
 * or JWKs.
 * If it failed, status_ holds the failure reason.
 *
 * Usage example:
 * EvpPkeyGetter e;
 * bssl::UniquePtr<EVP_PKEY> pkey =
 * e.createEvpPkeyFromStr(pem_formatted_public_key);
 * (You can use createEvpPkeyFromJwkRSA() or createEcKeyFromJwkEC() for JWKs)
 */
class EvpPkeyGetter : public WithStatus {
 public:
  // Create EVP_PKEY from PEM string
  bssl::UniquePtr<EVP_PKEY> createEvpPkeyFromStr(const std::string& pkey_pem) {
    // Header "-----BEGIN CERTIFICATE ---"and tailer "-----END CERTIFICATE ---"
    // should have been removed.
    std::string pkey_der;
    if (!absl::Base64Unescape(pkey_pem, &pkey_der) || pkey_der.empty()) {
      updateStatus(Status::JwksPemBadBase64);
      return nullptr;
    }
    auto rsa = bssl::UniquePtr<RSA>(
        RSA_public_key_from_bytes(castToUChar(pkey_der), pkey_der.length()));
    if (!rsa) {
      updateStatus(Status::JwksPemParseError);
      return nullptr;
    }
    return createEvpPkeyFromRsa(rsa.get());
  }

  bssl::UniquePtr<EVP_PKEY> createEvpPkeyFromJwkRSA(const std::string& n,
                                                    const std::string& e) {
    return createEvpPkeyFromRsa(createRsaFromJwk(n, e).get());
  }

  bssl::UniquePtr<EC_KEY> createEcKeyFromJwkEC(const std::string& x,
                                               const std::string& y) {
    bssl::UniquePtr<EC_KEY> ec_key(
        EC_KEY_new_by_curve_name(NID_X9_62_prime256v1));
    if (!ec_key) {
      updateStatus(Status::JwksEcCreateKeyFail);
      return nullptr;
    }
    bssl::UniquePtr<BIGNUM> bn_x = createBigNumFromBase64UrlString(x);
    bssl::UniquePtr<BIGNUM> bn_y = createBigNumFromBase64UrlString(y);
    if (!bn_x || !bn_y) {
      // EC public key field is missing or has parse error.
      updateStatus(Status::JwksEcParseError);
      return nullptr;
    }

    if (EC_KEY_set_public_key_affine_coordinates(ec_key.get(), bn_x.get(),
                                                 bn_y.get()) == 0) {
      updateStatus(Status::JwksEcParseError);
      return nullptr;
    }
    return ec_key;
  }

 private:
  bssl::UniquePtr<EVP_PKEY> createEvpPkeyFromRsa(RSA* rsa) {
    if (!rsa) {
      return nullptr;
    }
    bssl::UniquePtr<EVP_PKEY> key(EVP_PKEY_new());
    EVP_PKEY_set1_RSA(key.get(), rsa);
    return key;
  }

  bssl::UniquePtr<BIGNUM> createBigNumFromBase64UrlString(
      const std::string& s) {
    std::string s_decoded;
    if (!absl::WebSafeBase64Unescape(s, &s_decoded)) {
      return nullptr;
    }
    return bssl::UniquePtr<BIGNUM>(
        BN_bin2bn(castToUChar(s_decoded), s_decoded.length(), NULL));
  };

  bssl::UniquePtr<RSA> createRsaFromJwk(const std::string& n,
                                        const std::string& e) {
    bssl::UniquePtr<RSA> rsa(RSA_new());
    rsa->n = createBigNumFromBase64UrlString(n).release();
    rsa->e = createBigNumFromBase64UrlString(e).release();
    if (rsa->n == nullptr || rsa->e == nullptr) {
      // RSA public key field is missing or has parse error.
      updateStatus(Status::JwksRsaParseError);
      return nullptr;
    }
    if (BN_cmp_word(rsa->e, 3) != 0 && BN_cmp_word(rsa->e, 65537) != 0) {
      // non-standard key; reject it early.
      updateStatus(Status::JwksRsaParseError);
      return nullptr;
    }
    return rsa;
  }
};

Status extractJwkFromJwkRSA(const rapidjson::Value& jwk_json,
                            Jwks::Pubkey* jwk) {
  if (jwk->alg_specified_ &&
      (jwk->alg_.size() < 2 || jwk->alg_.compare(0, 2, "RS") != 0)) {
    return Status::JwksRSAKeyBadAlg;
  }

  if (!jwk_json.HasMember("n")) {
    return Status::JwksRSAKeyMissingN;
  }
  const auto& n_value = jwk_json["n"];
  if (!n_value.IsString()) {
    return Status::JwksRSAKeyBadN;
  }
  std::string n_str = n_value.GetString();

  if (!jwk_json.HasMember("e")) {
    return Status::JwksRSAKeyMissingE;
  }
  const auto& e_value = jwk_json["e"];
  if (!e_value.IsString()) {
    return Status::JwksRSAKeyBadE;
  }
  std::string e_str = e_value.GetString();

  EvpPkeyGetter e;
  jwk->evp_pkey_ = e.createEvpPkeyFromJwkRSA(n_str, e_str);
  return e.getStatus();
}

Status extractJwkFromJwkEC(const rapidjson::Value& jwk_json,
                           Jwks::Pubkey* jwk) {
  if (jwk->alg_specified_ && jwk->alg_ != "ES256") {
    return Status::JwksECKeyBadAlg;
  }

  if (!jwk_json.HasMember("x")) {
    return Status::JwksECKeyMissingX;
  }
  const auto& x_value = jwk_json["x"];
  if (!x_value.IsString()) {
    return Status::JwksECKeyBadX;
  }
  std::string x_str = x_value.GetString();

  if (!jwk_json.HasMember("y")) {
    return Status::JwksECKeyMissingY;
  }
  const auto& y_value = jwk_json["y"];
  if (!y_value.IsString()) {
    return Status::JwksECKeyBadY;
  }
  std::string y_str = y_value.GetString();

  EvpPkeyGetter e;
  jwk->ec_key_ = e.createEcKeyFromJwkEC(x_str, y_str);
  return e.getStatus();
}

Status extractJwk(const rapidjson::Value& jwk_json, Jwks::Pubkey* jwk) {
  // Check "kty" parameter, it should exist.
  // https://tools.ietf.org/html/rfc7517#section-4.1
  if (!jwk_json.HasMember("kty")) {
    return Status::JwksMissingKty;
  }
  const auto& kty_value = jwk_json["kty"];
  if (!kty_value.IsString()) {
    return Status::JwksBadKty;
  }
  jwk->kty_ = kty_value.GetString();

  // "kid" and "alg" are optional, if they do not exist, set them to empty.
  // https://tools.ietf.org/html/rfc7517#page-8
  if (jwk_json.HasMember("kid")) {
    const auto& kid_value = jwk_json["kid"];
    if (kid_value.IsString()) {
      jwk->kid_ = kid_value.GetString();
      jwk->kid_specified_ = true;
    }
  }
  if (jwk_json.HasMember("alg")) {
    const auto& alg_value = jwk_json["alg"];
    if (alg_value.IsString()) {
      jwk->alg_ = alg_value.GetString();
      jwk->alg_specified_ = true;
    }
  }

  // Extract public key according to "kty" value.
  // https://tools.ietf.org/html/rfc7518#section-6.1
  if (jwk->kty_ == "EC") {
    return extractJwkFromJwkEC(jwk_json, jwk);
  } else if (jwk->kty_ == "RSA") {
    return extractJwkFromJwkRSA(jwk_json, jwk);
  }
  return Status::JwksNotImplementedKty;
}

}  // namespace

JwksPtr Jwks::createFrom(const std::string& pkey, Type type) {
  JwksPtr keys(new Jwks());
  switch (type) {
    case Type::JWKS:
      keys->createFromJwksCore(pkey);
      break;
    case Type::PEM:
      keys->createFromPemCore(pkey);
      break;
    default:
      break;
  }
  return keys;
}

void Jwks::createFromPemCore(const std::string& pkey_pem) {
  keys_.clear();
  PubkeyPtr key_ptr(new Pubkey());
  EvpPkeyGetter e;
  key_ptr->evp_pkey_ = e.createEvpPkeyFromStr(pkey_pem);
  key_ptr->pem_format_ = true;
  updateStatus(e.getStatus());
  assert((key_ptr->evp_pkey_ == nullptr) == (e.getStatus() != Status::Ok));
  if (e.getStatus() == Status::Ok) {
    keys_.push_back(std::move(key_ptr));
  }
}

void Jwks::createFromJwksCore(const std::string& pkey_jwks) {
  keys_.clear();

  rapidjson::Document jwks_json;
  if (jwks_json.Parse(pkey_jwks.c_str()).HasParseError()) {
    updateStatus(Status::JwksParseError);
    return;
  }

  if (!jwks_json.HasMember("keys")) {
    updateStatus(Status::JwksNoKeys);
    return;
  }
  const auto& keys_value = jwks_json["keys"];
  if (!keys_value.IsArray()) {
    updateStatus(Status::JwksBadKeys);
    return;
  }

  for (auto key_it = keys_value.Begin(); key_it != keys_value.End(); ++key_it) {
    PubkeyPtr key_ptr(new Pubkey());
    Status status = extractJwk(*key_it, key_ptr.get());
    if (status == Status::Ok) {
      keys_.push_back(std::move(key_ptr));
    } else {
      updateStatus(status);
      break;
    }
  }

  if (keys_.empty()) {
    updateStatus(Status::JwksNoValidKeys);
  }
}

}  // namespace jwt_verify
}  // namespace google
