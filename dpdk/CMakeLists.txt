#set(dpdk_wholearchive_flags --whole-archive -Wl,--start-group -lrte_pmd_ena -Wl,--end-group -Wl)
#set(CMAKE_CXX_FLAGS ${dpdk_wholearchive_flags})
#set(CFLAGS ${dpdk_wholearchive_flags})

include(ExternalProject)

file(GLOB_RECURSE
    dpdk_sources
    ${CMAKE_SOURCE_DIR}/dpdk/*.h
    ${CMAKE_SOURCE_DIR}/dpdk/*.c
    #print_list(${dpdk_sources})
)

set(dpdk_configure_command make -C ${CMAKE_SOURCE_DIR}/dpdk/ O=${CMAKE_BINARY_DIR}/dpdk config T=x86_64-native-linuxapp-gcc)

ExternalProject_Add(dpdk_main
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/dpdk
    CONFIGURE_COMMAND ${dpdk_configure_command}
    BUILD_COMMAND make -C ${CMAKE_SOURCE_DIR}/dpdk/ O=${CMAKE_BINARY_DIR}/dpdk
    INSTALL_COMMAND echo "No install step for dpdk"
    STEP_TARGETS configure_dpdk
)

# Any files modified in dpdk needs running the CONFIGURE_COMMAND
#   (eg: if ena_ethdev.c is modified)
#
# The following step runs CONFIGURE_COMMAND when that happens

ExternalProject_Add_Step(dpdk_main configure_dpdk
    COMMAND ${dpdk_configure_command}
    COMMENT "configure dpdk"
    DEPENDS ${dpdk_sources}
    DEPENDERS configure # configure step of project dpdk depends on this step
    DEPENDEES update    # this step depends on update step
)

add_library(dpdk STATIC IMPORTED)
set_target_properties(dpdk PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/dpdk/lib/libdpdk.a)
