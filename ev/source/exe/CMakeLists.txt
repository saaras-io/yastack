include(${CMAKE_SOURCE_DIR}/ev/CMakeSettings.txt)
include(${CMAKE_SOURCE_DIR}/ev/CMakeIncludes.txt)

file(GLOB_RECURSE
    sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
)

#TODO ev/external/libeven/.lib/libevent.a still under source folders

set(ev_src_dir      			${CMAKE_SOURCE_DIR}/ev)
set(ev_bin_dir      			${CMAKE_BINARY_DIR}/ev)
set(ev_bin_external_dir      	${CMAKE_BINARY_DIR}/external)

set(dpdk_lib_path				${CMAKE_BINARY_DIR}/dpdk/lib/)
set(fs_lib_path					${CMAKE_BINARY_DIR}/lib/fs/)
set(fs_event_path				${CMAKE_BINARY_DIR}/fs/app/libevent/lib/)

set(rsync_cmd                   rsync -auv)

link_directories(
    ${fs_lib_path}
    ${dpdk_lib_path}
    ${CMAKE_SOURCE_DIR}/ev/external/grpc/third_party/bloaty/tests/testdata/linux-x86_64
    ${CMAKE_SOURCE_DIR}/ev/external/libevent/.libs
    ${CMAKE_SOURCE_DIR}/ev/external/luajit-2.0/src
    ${CMAKE_SOURCE_DIR}/ev/external/xxHash
    ${CMAKE_SOURCE_DIR}/ev/external/grpc_old/third_party/bloaty/tests/testdata/linux-x86_64
    ${CMAKE_SOURCE_DIR}/fs/tools/libnetgraph
    ${CMAKE_SOURCE_DIR}/fs/tools/libutil
    ${CMAKE_SOURCE_DIR}/fs/tools/libxo
    ${CMAKE_SOURCE_DIR}/fs/tools/compat
    ${CMAKE_SOURCE_DIR}/fs/tools/libmemstat
	${CMAKE_SOURCE_DIR}/ev/external/libcircllhist/src/
	# Libraries under binary directory
    ${CMAKE_BINARY_DIR}/ev/api/
    ${CMAKE_BINARY_DIR}/ev/source/server/
	${CMAKE_BINARY_DIR}/ev/source/common/ssl
	${CMAKE_BINARY_DIR}/ev/source/common/tcp
	${CMAKE_BINARY_DIR}/ev/source/common/upstream
	${CMAKE_BINARY_DIR}/ev/source/common/router
	${CMAKE_BINARY_DIR}/ev/source/common/grpc
	${CMAKE_BINARY_DIR}/ev/source/common/config
	${CMAKE_BINARY_DIR}/ev/source/common/http
	${CMAKE_BINARY_DIR}/ev/source/common/common/
	${CMAKE_BINARY_DIR}/ev/external/lightstep-tracer-cpp/
	${CMAKE_BINARY_DIR}/ev/external/opentracing-cpp/
	${CMAKE_BINARY_DIR}/ev/source/common/network
	${CMAKE_BINARY_DIR}/ev/external/boringssl/ssl/
	${CMAKE_BINARY_DIR}/ev/external/boringssl/crypto/
	${CMAKE_BINARY_DIR}/ev/source/common/protobuf
	${CMAKE_BINARY_DIR}/ev/source/extensions/
	${CMAKE_BINARY_DIR}/ev/source/common/tcp_proxy/
	${CMAKE_BINARY_DIR}/ev/source/server/http/
	${CMAKE_BINARY_DIR}/ev/source/common/access_log/
	${CMAKE_BINARY_DIR}/ev/source/common/secret/
	${CMAKE_BINARY_DIR}/ev/external/jwt_verify_lib/
	${CMAKE_BINARY_DIR}/ev/external/fmt-4.1.0/fmt/
)

add_library(sig_action STATIC ${CMAKE_CURRENT_SOURCE_DIR}/signal_action.cc)
set_target_properties(sig_action PROPERTIES LINKER_LANGUAGE CXX)

add_library(${project_name}-lib ${sources})
add_executable(${project_name} ${sources})
set_property(TARGET ${project_name} PROPERTY CXX_STANDARD 14)

add_custom_command(TARGET ${project_name}
    PRE_BUILD
    COMMAND echo " --- CUSTOM COMMAND ---"
    COMMAND touch ${CMAKE_SOURCE_DIR}/ev/source/exe/main.cc
    )

# Note that while inspecting the descriptor pool for protobuf in validateProtoDescriptors()
# some of the descriptors came up missing.
# This resulted in this call Protobuf::DescriptorPool::generated_pool()->FindMethodByName(method) returning nullptr
# The issue here https://github.com/google/protobuf/issues/4221 talks about how the linker drops
# some symbols.
# The fix is to provide linker flags to ensure that linker doesn't drop symbols
# Description about this can be found here
# https://cmake.org/pipermail/cmake/2007-September/016238.html
# and here
# https://www.mail-archive.com/cmake@cmake.org/msg01890.html
# After adding the -Wl,-whole-archive -lprotos-api -Wl,-no-whole-archive
# the linker doesn't drop the symbols.
# Also note that CMake leaves the line alone by directly sending it to the linker since
# it starts with a '-'
target_link_libraries(${project_name}
    -Wl,-whole-archive -lev-source-server -Wl,-no-whole-archive
    -Wl,-whole-archive -lprotos-api -Wl,-no-whole-archive
    -Wl,-whole-archive -lfstack -Wl,-no-whole-archive
    -Wl,-whole-archive -ldpdk -Wl,-no-whole-archive
    -Wl,-whole-archive -ldl -Wl,-no-whole-archive
    -Wl,-whole-archive -lnuma -Wl,-no-whole-archive
    xxhash
    ev-source-common-memory
    ev-source-common-singleton
    ev-source-common-thread_local
    -Wl,-whole-archive -lev-source-common-ssl -Wl,-no-whole-archive
    ev-source-common-ratelimit
    -Wl,-whole-archive -lev-source-common-tcp -Wl,-no-whole-archive
    -Wl,-whole-archive -lev-source-common-upstream -Wl,-no-whole-archive
    #redis
    ev-source-common-html
    ev-source-common-profiler
    ev-source-common-event
    #network
    ev-source-common-json
	yaml-cpp
    ev-source-common-filesystem
    ev-source-common-api
    -Wl,-whole-archive -lev-source-common-router -Wl,-no-whole-archive
    #mongo
    #dynamo
    -Wl,-whole-archive -lev-source-common-grpc -Wl,-no-whole-archive
    ev-source-common-stats
    -Wl,-whole-archive -lev-source-common-config -Wl,-no-whole-archive
    -Wl,-whole-archive -lev-source-common-http -Wl,-no-whole-archive
    -Wl,-whole-archive -lcommon -Wl,-no-whole-archive
    #filter
    #ext_authz
    ev-external-http-parser
    ev-source-common-request_info
    ev-source-common-tracing
    -Wl,-whole-archive -llightstep_tracer -Wl,-no-whole-archive
	-Wl,-whole-archive -lopentracing -Wl,-no-whole-archive
    ev-source-common-runtime
    -Wl,-whole-archive -lev-source-common-network -Wl,-no-whole-archive
    nghttp2
    -Wl,-whole-archive -lssl -Wl,-no-whole-archive
    -Wl,-whole-archive -lcrypto -Wl,-no-whole-archive
    # posix real-time extensions
    rt
    -Wl,-whole-archive -lev-source-common-protobuf -Wl,-no-whole-archive
    protobuf
	absl_utility
	absl_time
	absl_memory
	absl_synchronization
	absl_algorithm
	absl_bad_optional_access
	absl_bad_any_cast
	absl_span
	absl_optional
	absl_any
	absl_strings
	absl_int128
	absl_numeric
	absl_malloc_internal
	absl_throw_delegate
	absl_spinlock_wait
	absl_base
	absl_dynamic_annotations
	absl_meta
	absl_stacktrace
	absl_leak_check
	absl_debugging
    cares
    ev-source-common-buffer
    pthread
    #event_pthreads
    event_core
    event
    event_extra
    -Wl,-whole-archive -lev-source-extensions -Wl,-no-whole-archive
    -Wl,-whole-archive -lluajit -Wl,-no-whole-archive
	ev-external-grpc-httpjson-transcoding
	ev-source-common-compressor
	ev-source-common-decompressor
    z
    -lcircllhist
    -lev-source-common-tcp_proxy
    -lev-source-server-http
    -lev-source-common-access_log
    -lev-source-common-secret
    -lev-external-jwt_verify_lib
    grpc++
    -lfmt
    )

#set_target_properties(${project_name}   PROPERTIES LINK_FLAGS "-static" )
