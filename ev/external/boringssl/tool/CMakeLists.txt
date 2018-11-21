#include_directories(../include)
#
#link_directories(${CMAKE_BINARY_DIR}/lib/fs/)
#link_directories(${CMAKE_BINARY_DIR}/dpdk/lib/)
#add_executable(
#  bssl
#
#  args.cc
#  ciphers.cc
#  client.cc
#  const.cc
#  digest.cc
#  file.cc
#  generate_ed25519.cc
#  genrsa.cc
#  pkcs12.cc
#  rand.cc
#  server.cc
#  sign.cc
#  speed.cc
#  tool.cc
#  transport_common.cc
#)
#
#if (APPLE OR WIN32 OR ANDROID)
#  target_link_libraries(bssl ssl crypto fstack dpdk dl numa)
#else()
#  find_library(FOUND_LIBRT rt)
#  if (FOUND_LIBRT)
#    target_link_libraries(bssl ssl crypto -lrt fstack dpdk dl numa)
#  else()
#    target_link_libraries(bssl ssl crypto fstack dpdk dl numa)
#  endif()
#endif()
