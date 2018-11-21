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

#include "gtest/gtest.h"
#include "jwt_verify_lib/verify.h"
#include "src/test_common.h"

namespace google {
namespace jwt_verify {
namespace {

// JWT with
// Header:  {"alg":"RS256","typ":"JWT"}
// Payload:
// {"iss":"https://example.com","sub":"test@example.com","exp":
// 9223372036854775806,"nbf":10}
const std::string JwtText =
    "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9."
    "eyJpc3MiOiJodHRwczovL2V4YW1wbGUuY29tIiwic3ViIjoidGVzdEBleGFtcGxlLmNvbSIsIm"
    "V4"
    "cCI6OTIyMzM3MjAzNjg1NDc3NTgwNiwibmJmIjoxMH0K."
    "digk0Fr_IdcWgJNVyeVDw2dC1cQG6LsHwg5pIN93L4_"
    "xhEDI3ZFoZ8aE44kvQHWLicnHDlhELqtF-"
    "TqxrhfnitpLE7jiyknSu6NVXxtRBcZ3dOTKryVJDvDXcYXOaaP8infnh82loHfhikgg1xmk9"
    "rcH50jtc3BkxWNbpNgPyaAAE2tEisIInaxeX0gqkwiNVrLGe1hfwdtdlWFL1WENGlyniQBvB"
    "Mwi8DgG_F0eyFKTSRWoaNQQXQruEK0YIcwDj9tkYOXq8cLAnRK9zSYc5-"
    "15Hlzfb8eE77pID0HZN-Axeui4IY22I_kYftd0OEqlwXJv_v5p6kNaHsQ9QbtAkw";

TEST(VerifyExpTest, Expired) {
  Jwt jwt;
  Jwks jwks;
  EXPECT_EQ(jwt.parseFromString(JwtText), Status::Ok);
  EXPECT_EQ(verifyJwt(jwt, jwks, 9223372036854775807), Status::JwtExpired);
}

TEST(VerifyExpTest, NotBefore) {
  Jwt jwt;
  Jwks jwks;
  EXPECT_EQ(jwt.parseFromString(JwtText), Status::Ok);
  EXPECT_EQ(verifyJwt(jwt, jwks, 9), Status::JwtNotYetValid);
}

}  // namespace
}  // namespace jwt_verify
}  // namespace google
