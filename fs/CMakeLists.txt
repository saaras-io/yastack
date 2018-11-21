#set(fs_wholearchive_flags --whole-archive -Wl,--start-group -lrte_pmd_ena -Wl,--end-group -Wl)
#set(CMAKE_CXX_FLAGS ${fs_wholearchive_flags})
#set(CFLAGS ${fs_wholearchive_flags})

include(ExternalProject)

## (1)  Bulid file list for dependency tracking - anytime a file source changes, we can rebuild
file(GLOB_RECURSE
    fs_sources
    ${CMAKE_SOURCE_DIR}/fs/lib/*.h
    ${CMAKE_SOURCE_DIR}/fs/lib/*.c
    #print_list(${fs_sources})
)

file(GLOB_RECURSE
    fs_example_sources
    ${CMAKE_SOURCE_DIR}/fs/example/*.h
    ${CMAKE_SOURCE_DIR}/fs/example/*.c
)

file(GLOB_RECURSE
    fs_tools_sources
    ${CMAKE_SOURCE_DIR}/fs/tools/*.h
    ${CMAKE_SOURCE_DIR}/fs/tools/*.c
)

## (2)  Makefile(s) internally references FF_DPDK FF_INCLUDES FF_LIB_INCLUDE
set (fs-common-make-args
    FF_DPDK=${CMAKE_BINARY_DIR}/dpdk    
    CMAKE_BUILD_CURDIR=${CMAKE_BINARY_DIR}/fs 
    FF_PATH=${CMAKE_SOURCE_DIR}/fs)

set (dpdk-headers
    FF_INCLUDES=-I${CMAKE_BINARY_DIR}/dpdk/include)

set (dpdk-libs
    DPDK_LIBS=${CMAKE_BINARY_DIR}/dpdk/lib)

set (openssl-headers
    OPENSSL_HEADERS=-I${CMAKE_SOURCE_DIR}/ev/external/boringssl/include)

set (openssl-libs
     OPENSSL_LIBS=${CMAKE_BINARY_DIR}/ev/external/boringssl/crypto/)

set (fs-includes ${dpdk-headers} ${openssl-headers})

set (fs-app-libs
    FF_LIBS=${CMAKE_BINARY_DIR}/lib/fs/)

set (fs-app-includes
    FF_LIB_INCLUDES=${CMAKE_SOURCE_DIR}/fs/lib)

# (3) Define targets based on external make
ExternalProject_Add(fs
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/fs/lib
    CONFIGURE_COMMAND echo "No configure command for fs"
    BUILD_COMMAND make -C ${CMAKE_SOURCE_DIR}/fs/lib/ ${fs-common-make-args} ${fs-includes}
    INSTALL_COMMAND echo "No install step for fs"
    STEP_TARGETS fs_step_recompile fs_step_reconfigure
    #fs_example
)

# For first-time invocations 
ExternalProject_Add_Step(fs fs_step_reconfigure
    COMMAND make -C ${CMAKE_SOURCE_DIR}/fs/lib/ ${fs-common-make-args} ${fs-includes}
    DEPENDEES update
    )

# Force recompilation anytime ${fs_sources} change
ExternalProject_Add_Step(fs fs_step_recompile
    DEPENDS ${fs_sources} fs-fs_step_reconfigure
    COMMAND make -C ${CMAKE_SOURCE_DIR}/fs/lib/ ${fs-common-make-args} ${fs-includes}
    DEPENDEES update
    )

#ExternalProject_Add_Step(fs fs_example
#    DEPENDS ${fs_example_sources}
#    COMMAND make -C ${CMAKE_SOURCE_DIR}/fs/example/ ${fs-common-make-args} ${fs-app-libs} ${fs-app-includes} ${openssl-libs}
#    DEPENDEES update
#    )
#

# (4) Setup target inter-dependencies and compile app code. Attach commands to newly defined targets.

# Define custom target that depends and is re-compiled every time 'fs' is compiled
# Note the ALL flag that adds this target to the default target (i.e. when make is
# invoked without any arguments)
add_custom_target(fs-app
    ALL
    DEPENDS fs
    )

add_custom_command(TARGET fs-app
    POST_BUILD
    COMMAND make -C ${CMAKE_SOURCE_DIR}/fs/example/ ${fs-app-libs} ${fs-app-includes} ${openssl-libs} ${dpdk-libs}
    COMMAND find ${CMAKE_SOURCE_DIR}/fs/example/ -name helloworld -exec mv {} ${CMAKE_BINARY_DIR}/lib/fs/ '\;'
    COMMAND find ${CMAKE_SOURCE_DIR}/fs/example/ -name helloworld_epoll -exec mv {} ${CMAKE_BINARY_DIR}/lib/fs/ '\;'
    )

add_custom_target(fs-examples
    ALL
    DEPENDS fs
    )

add_custom_command(TARGET fs-examples
    POST_BUILD
    COMMAND make -C ${CMAKE_SOURCE_DIR}/fs/example/ ${fs-app-libs} ${fs-app-includes} ${openssl-libs} ${dpdk-libs}
    COMMAND find ${CMAKE_SOURCE_DIR}/fs/example/ -name helloworld -exec mv {} ${CMAKE_BINARY_DIR}/lib/fs/ '\;'
    COMMAND find ${CMAKE_SOURCE_DIR}/fs/example/ -name helloworld_epoll -exec mv {} ${CMAKE_BINARY_DIR}/lib/fs/ '\;'
    )

add_custom_target(fs-tools
    ALL
    DEPENDS ${fs_sources} ${fs_tools_sources}
    )

#TODO: cleanup the *.o and binaries during distclean
add_custom_command(TARGET fs-tools
    POST_BUILD
    COMMAND make -C ${CMAKE_SOURCE_DIR}/fs/tools/ ${fs-common-make-args} ${fs-app-libs} ${fs-app-includes} ${openssl-libs} ${dpdk-libs}
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/fs/tools
    COMMAND find ${CMAKE_SOURCE_DIR}/fs/tools/ -type f -executable -exec cp {} ${CMAKE_BINARY_DIR}/fs/tools/ '\;'
    #COMMAND find ${CMAKE_SOURCE_DIR}/fs/tools/ -name "*.o" -exec mv {} ${CMAKE_BINARY_DIR}/fs/tools/ '\;'
    )


# (5) Post build, move files
add_custom_command(TARGET fs
    POST_BUILD
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/lib/
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/lib/fs/
    COMMAND find ${CMAKE_SOURCE_DIR}/fs/lib/ -name libfstack.a -exec mv -f {} ${CMAKE_BINARY_DIR}/lib/fs/ '\;'
    COMMAND find ${CMAKE_SOURCE_DIR}/fs/example/ -name helloworld -exec mv -f {} ${CMAKE_BINARY_DIR}/lib/fs/ '\;'
    COMMAND find ${CMAKE_SOURCE_DIR}/fs/example/ -name helloworld_epoll -exec mv -f {} ${CMAKE_BINARY_DIR}/lib/ '\;'
    COMMENT "post-fs-build commands"
    )

add_custom_target(fs-strip-symbols
    DEPENDS fs  fs-tools    fs-app  fs-examples)

add_custom_command(TARGET fs-strip-symbols
    POST_BUILD
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/bin/debug/symbols
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/bin/debug/unstripped
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/bin/debug/stripped
    # find in all directories except ${CMAKE_BINARY_DIR}/bin
    COMMAND find ${CMAKE_BINARY_DIR} -path ${CMAKE_BINARY_DIR}/bin -prune -o -type f -executable -exec cp {} ${CMAKE_BINARY_DIR}/bin/debug/unstripped '\;'
    COMMAND find ${CMAKE_BINARY_DIR} -path ${CMAKE_BINARY_DIR}/bin -prune -o -type f -executable -exec cp {} ${CMAKE_BINARY_DIR}/bin/debug/stripped '\;'
    COMMAND find ${CMAKE_BINARY_DIR}/bin/debug/stripped -type f -executable -exec ${CMAKE_SOURCE_DIR}/scripts/strip_symbols.sh {} ${CMAKE_BINARY_DIR}/bin/debug/symbols '\;'
    COMMENT "fs-strip-symbols"
    )

add_subdirectory(app/libevent)

add_dependencies(event fs)
add_dependencies(event_core fs)
add_dependencies(event_extra fs)
#add_dependencies(bench fs)
add_dependencies(hello-world fs)
