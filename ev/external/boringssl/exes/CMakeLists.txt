# TODO: Build these after fstack and dpdk have been built.
# We define sockets that use dpdk, these libraries use those sockets to build a binary
# The dpdk and fstack libraries will be needed to build them.

# bssl

include_directories(../include)
include_directories(${CMAKE_SOURCE_DIR}/ev/external/boringssl/third_party/googletest/include/)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fexceptions -frtti")

link_directories(${CMAKE_BINARY_DIR}/lib/fs/)
link_directories(${CMAKE_BINARY_DIR}/dpdk/lib/)
add_executable(
  bssl

  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/args.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/ciphers.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/client.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/const.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/digest.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/file.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/generate_ed25519.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/genrsa.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/pkcs12.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/rand.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/server.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/sign.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/speed.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/tool.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/tool/transport_common.cc
)

if (APPLE OR WIN32 OR ANDROID)
  target_link_libraries(bssl ssl crypto fstack dpdk dl numa)
else()
  find_library(FOUND_LIBRT rt)
  if (FOUND_LIBRT)
    target_link_libraries(bssl ssl crypto -lrt fstack dpdk dl numa)
  else()
    target_link_libraries(bssl ssl crypto fstack dpdk dl numa)
  endif()
endif()

#bssl_shim

include_directories(../include)

link_directories(${CMAKE_BINARY_DIR}/lib/fs/)
link_directories(${CMAKE_BINARY_DIR}/dpdk/lib/)
add_executable(
  bssl_shim

  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/ssl/test/async_bio.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/ssl/test/bssl_shim.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/ssl/test/packeted_bio.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/ssl/test/test_config.cc

  $<TARGET_OBJECTS:test_support>
)

target_link_libraries(bssl_shim ssl crypto fstack dpdk dl numa)


# decript_test

link_directories(${CMAKE_BINARY_DIR}/lib/fs/)
link_directories(${CMAKE_BINARY_DIR}/dpdk/lib/)
add_executable(
  decrepit_test

  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/decrepit/ripemd/ripemd_test.cc
  ${CMAKE_SOURCE_DIR}/ev/external/boringssl/decrepit/cfb/cfb_test.cc

  $<TARGET_OBJECTS:boringssl_gtest_main>
  $<TARGET_OBJECTS:test_support>
)

target_link_libraries(decrepit_test crypto decrepit boringssl_gtest fstack dpdk dl numa)
if (WIN32)
  target_link_libraries(decrepit_test ws2_32)
endif()


# run_tests

add_custom_target(
    run_tests
    COMMAND ${GO_EXECUTABLE} run util/all_tests.go -build-dir
            ${CMAKE_BINARY_DIR}
    COMMAND cd ssl/test/runner &&
            ${GO_EXECUTABLE} test -shim-path $<TARGET_FILE:bssl_shim>
              ${RUNNER_ARGS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS all_tests bssl_shim
    ${MAYBE_USES_TERMINAL})
