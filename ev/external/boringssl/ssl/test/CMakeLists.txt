include_directories(../../include)

link_directories(${CMAKE_BINARY_DIR}/lib/fs/)
link_directories(${CMAKE_BINARY_DIR}/dpdk/lib/)
add_executable(
  bssl_shim

  async_bio.cc
  bssl_shim.cc
  packeted_bio.cc
  test_config.cc

  $<TARGET_OBJECTS:test_support>
)

target_link_libraries(bssl_shim ssl crypto fstack dpdk dl numa)
