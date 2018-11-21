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

#pragma once

#include <string>

namespace google {
namespace jwt_verify {

/**
 * Define the Jwt verification error status.
 */
enum class Status {
  Ok = 0,

  // Jwt errors:

  // Jwt missing.
  JwtMissed,

  // Jwt not valid yet.
  JwtNotYetValid,

  // Jwt expired.
  JwtExpired,

  // JWT is not in the form of Header.Payload.Signature
  JwtBadFormat,

  // Jwt header is an invalid Base64url input or an invalid JSON.
  JwtHeaderParseError,

  // "alg" in the header is not a string.
  JwtHeaderBadAlg,

  // Value of "alg" in the header is invalid.
  JwtHeaderNotImplementedAlg,

  // "kid" in the header is not a string.
  JwtHeaderBadKid,

  // Jwt payload is an invalid Base64url input or an invalid JSON.
  JwtPayloadParseError,

  // Jwt signature is an invalid Base64url input.
  JwtSignatureParseError,

  // Issuer is not configured.
  JwtUnknownIssuer,

  // Audience is not allowed.
  JwtAudienceNotAllowed,

  // Jwt verification fails.
  JwtVerificationFail,

  // Jwks errors

  // Jwks is an invalid JSON.
  JwksParseError,

  // Jwks does not have "keys".
  JwksNoKeys,

  // "keys" in Jwks is not an array.
  JwksBadKeys,

  // Jwks doesn't have any valid public key.
  JwksNoValidKeys,

  // Jwks doesn't have key to match kid or alg from Jwt.
  JwksKidAlgMismatch,

  // Jwks PEM public key is an invalid Base64.
  JwksPemBadBase64,

  // Jwks PEM public key parse error.
  JwksPemParseError,

  // "n" or "e" field of a Jwk RSA is missing or has a parse error.
  JwksRsaParseError,

  // Failed to create a EC_KEY object.
  JwksEcCreateKeyFail,

  // "x" or "y" field of a Jwk EC is missing or has a parse error.
  JwksEcParseError,

  // Failed to fetch public key
  JwksFetchFail,

  // "kty" is missing in "keys".
  JwksMissingKty,
  // "kty" is not string type in "keys".
  JwksBadKty,
  // "kty" is not supported in "keys".
  JwksNotImplementedKty,

  // "alg" is not started with "RS" for a RSA key
  JwksRSAKeyBadAlg,
  // "n" field is missing for a RSA key
  JwksRSAKeyMissingN,
  // "n" field is not string for a RSA key
  JwksRSAKeyBadN,
  // "e" field is missing for a RSA key
  JwksRSAKeyMissingE,
  // "e" field is not string for a RSA key
  JwksRSAKeyBadE,

  // "alg" is not "ES256" for an EC key
  JwksECKeyBadAlg,
  // "x" field is missing for an EC key
  JwksECKeyMissingX,
  // "x" field is not string for an EC key
  JwksECKeyBadX,
  // "y" field is missing for an EC key
  JwksECKeyMissingY,
  // "y" field is not string for an EC key
  JwksECKeyBadY,
};

/**
 * Convert enum status to string.
 * @param status is the enum status.
 * @return the string status.
 */
std::string getStatusString(Status status);

/**
 * Base class to keep the status that represents "OK" or the first failure.
 */
class WithStatus {
 public:
  WithStatus() : status_(Status::Ok) {}

  /**
   * Get the current status.
   * @return the enum status.
   */
  Status getStatus() const { return status_; }

 protected:
  void updateStatus(Status status) {
    // Only keep the first failure
    if (status_ == Status::Ok) {
      status_ = status;
    }
  }

 private:
  // The internal status.
  Status status_;
};

}  // namespace jwt_verify
}  // namespace google
