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

#include <map>

#include "jwt_verify_lib/status.h"

namespace google {
namespace jwt_verify {

std::string getStatusString(Status status) {
  switch (status) {
    case Status::Ok:
      return "OK";
    case Status::JwtMissed:
      return "Jwt is missing";
    case Status::JwtNotYetValid:
      return "Jwt not yet valid";
    case Status::JwtExpired:
      return "Jwt is expired";
    case Status::JwtBadFormat:
      return "Jwt is not in the form of Header.Payload.Signature";
    case Status::JwtHeaderParseError:
      return "Jwt header is an invalid Base64url input or an invalid JSON";
    case Status::JwtHeaderBadAlg:
      return "Jwt header [alg] field is not a string";
    case Status::JwtHeaderNotImplementedAlg:
      return "Jwt header [alg] field value is invalid";
    case Status::JwtHeaderBadKid:
      return "Jwt header [kid] field is not a string";
    case Status::JwtPayloadParseError:
      return "Jwt payload is an invalid Base64 or an invalid JSON";
    case Status::JwtSignatureParseError:
      return "Jwt signature is an invalid Base64";
    case Status::JwtUnknownIssuer:
      return "Jwt issuer is not configured";
    case Status::JwtAudienceNotAllowed:
      return "Audiences in Jwt are not allowed";
    case Status::JwtVerificationFail:
      return "Jwt verification fails";

    case Status::JwksParseError:
      return "Jwks is an invalid JSON";
    case Status::JwksNoKeys:
      return "Jwks does not have [keys] field";
    case Status::JwksBadKeys:
      return "[keys] in Jwks is not an array";
    case Status::JwksNoValidKeys:
      return "Jwks doesn't have any valid public key";
    case Status::JwksKidAlgMismatch:
      return "Jwks doesn't have key to match kid or alg from Jwt";
    case Status::JwksPemBadBase64:
      return "Jwks PEM public key is an invalid Base64";
    case Status::JwksPemParseError:
      return "Jwks PEM public key parse error";
    case Status::JwksRsaParseError:
      return "Jwks RSA [n] or [e] field is missing or has a parse error";
    case Status::JwksEcCreateKeyFail:
      return "Jwks EC create key fail";
    case Status::JwksEcParseError:
      return "Jwks EC [x] or [y] field is missing or has a parse error.";
    case Status::JwksFetchFail:
      return "Jwks remote fetch is failed";

    case Status::JwksMissingKty:
      return "[kty] is missing in [keys]";
    case Status::JwksBadKty:
      return "[kty] is bad in [keys]";
    case Status::JwksNotImplementedKty:
      return "[kty] is not supported in [keys]";

    case Status::JwksRSAKeyBadAlg:
      return "[alg] is not started with [RS] for a RSA key";
    case Status::JwksRSAKeyMissingN:
      return "[n] field is missing for a RSA key";
    case Status::JwksRSAKeyBadN:
      return "[n] field is not string for a RSA key";
    case Status::JwksRSAKeyMissingE:
      return "[e] field is missing for a RSA key";
    case Status::JwksRSAKeyBadE:
      return "[e] field is not string for a RSA key";

    case Status::JwksECKeyBadAlg:
      return "[alg] is not [ES256] for an EC key";
    case Status::JwksECKeyMissingX:
      return "[x] field is missing for an EC key";
    case Status::JwksECKeyBadX:
      return "[x] field is not string for an EC key";
    case Status::JwksECKeyMissingY:
      return "[y] field is missing for an EC key";
    case Status::JwksECKeyBadY:
      return "[y] field is not string for an EC key";
  };
  return "";
}

}  // namespace jwt_verify
}  // namespace google
