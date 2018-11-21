include(${CMAKE_SOURCE_DIR}/ev/CMakeSettings.txt)
include(${CMAKE_SOURCE_DIR}/ev/CMakeTestIncludes.txt)

set(project_name    ev-test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++1y -Wl,-V")
add_definitions(${CMAKE_C_FLAGS})

set(dpdk_lib_path				${CMAKE_BINARY_DIR}/dpdk/lib/)
set(fs_lib_path					${CMAKE_BINARY_DIR}/lib/fs/)
set(test_common_lib_path        ${CMAKE_BINARY_DIR}/ev/test/test_common/)
set(ev_test_binary_root_dir		${CMAKE_BINARY_DIR}/ev/test/)
set(sig_action_lib_path         ${CMAKE_BINARY_DIR}/ev/source/exe/)

add_subdirectory(proto)
add_subdirectory(test_common/)
add_subdirectory(server/http/)
add_subdirectory(server/)
add_subdirectory(server/config_validation/)
add_subdirectory(integration/)
add_subdirectory(config/)
add_subdirectory(extensions/)
add_subdirectory(extensions/health_checkers/redis/)
add_subdirectory(extensions/access_loggers/file/)
add_subdirectory(extensions/access_loggers/http_grpc/)
add_subdirectory(extensions/tracers/dynamic_ot/)
add_subdirectory(extensions/tracers/common/ot/)
add_subdirectory(extensions/tracers/lightstep/)
add_subdirectory(extensions/tracers/zipkin/)
add_subdirectory(extensions/stats_sinks/dog_statsd/)
add_subdirectory(extensions/stats_sinks/statsd/)
add_subdirectory(extensions/stats_sinks/metrics_service/)
add_subdirectory(extensions/stats_sinks/common/statsd/)
add_subdirectory(extensions/filters/http/ratelimit/)
add_subdirectory(extensions/filters/http/fault/)
add_subdirectory(extensions/filters/http/lua/)
add_subdirectory(extensions/filters/http/gzip/)
add_subdirectory(extensions/filters/http/health_check/)
add_subdirectory(extensions/filters/http/grpc_json_transcoder/)
add_subdirectory(extensions/filters/http/rbac/)
add_subdirectory(extensions/filters/http/ext_authz/)
add_subdirectory(extensions/filters/http/grpc_web/)
add_subdirectory(extensions/filters/http/router/)
add_subdirectory(extensions/filters/http/cors/)
add_subdirectory(extensions/filters/http/jwt_authn/)
add_subdirectory(extensions/filters/http/buffer/)
add_subdirectory(extensions/filters/http/dynamo/)
add_subdirectory(extensions/filters/http/ip_tagging/)
add_subdirectory(extensions/filters/http/grpc_http1_bridge/)
add_subdirectory(extensions/filters/http/squash/)
add_subdirectory(extensions/filters/listener/tls_inspector/)
add_subdirectory(extensions/filters/listener/proxy_protocol/)
add_subdirectory(extensions/filters/common/lua/)
add_subdirectory(extensions/filters/common/rbac/)
add_subdirectory(extensions/filters/common/ext_authz/)
add_subdirectory(extensions/filters/network/ratelimit/)
add_subdirectory(extensions/filters/network/ext_authz/)
add_subdirectory(extensions/filters/network/http_connection_manager/)
add_subdirectory(extensions/filters/network/client_ssl_auth/)
add_subdirectory(extensions/filters/network/mongo_proxy/)
add_subdirectory(extensions/filters/network/redis_proxy/)
add_subdirectory(extensions/filters/network/tcp_proxy/)
add_subdirectory(fuzz/)
add_subdirectory(coverage/gcc_only_test/)
add_subdirectory(common/thread_local/)
add_subdirectory(common/http/)
add_subdirectory(common/http/http2/)
add_subdirectory(common/http/http1/)
add_subdirectory(common/ratelimit/)
add_subdirectory(common/grpc/)
add_subdirectory(common/stats/)
add_subdirectory(common/filesystem/)
add_subdirectory(common/config/)
add_subdirectory(common/runtime/)
add_subdirectory(common/upstream/)
add_subdirectory(common/request_info/)
add_subdirectory(common/access_log/)
add_subdirectory(common/tracing/)
add_subdirectory(common/router/)
add_subdirectory(common/buffer/)
add_subdirectory(common/common/)
add_subdirectory(common/html/)
add_subdirectory(common/network/)
add_subdirectory(common/tcp_proxy/)
add_subdirectory(common/protobuf/)
add_subdirectory(common/singleton/)
add_subdirectory(common/event/)
add_subdirectory(common/compressor/)
add_subdirectory(common/ssl/)
add_subdirectory(common/decompressor/)
add_subdirectory(common/api/)
add_subdirectory(common/json/)
add_subdirectory(tools/schema_validator/)
add_subdirectory(tools/router_check/)
add_subdirectory(tools/router_check/json/)
add_subdirectory(tools/config_load_check/)
add_subdirectory(config_test/)
add_subdirectory(exe/)
add_subdirectory(mocks/thread_local/)
add_subdirectory(mocks/http/)
add_subdirectory(mocks/ratelimit/)
add_subdirectory(mocks/)
add_subdirectory(mocks/grpc/)
add_subdirectory(mocks/stats/)
add_subdirectory(mocks/filesystem/)
add_subdirectory(mocks/server/)
add_subdirectory(mocks/config/)
add_subdirectory(mocks/runtime/)
add_subdirectory(mocks/upstream/)
add_subdirectory(mocks/request_info/)
add_subdirectory(mocks/access_log/)
add_subdirectory(mocks/tracing/)
add_subdirectory(mocks/local_info/)
add_subdirectory(mocks/router/)
add_subdirectory(mocks/init/)
add_subdirectory(mocks/buffer/)
add_subdirectory(mocks/network/)
add_subdirectory(mocks/event/)
add_subdirectory(mocks/ssl/)
add_subdirectory(mocks/api/)

file(GLOB
    sources
    ${CMAKE_SOURCE_DIR}/ev/test/*.h
    ${CMAKE_SOURCE_DIR}/ev/test/*.c
    ${CMAKE_SOURCE_DIR}/ev/test/*.cc
)

link_directories(
    ${fs_lib_path}
    ${dpdk_lib_path}
    ${CMAKE_BINARY_DIR}/dpdk/lib/
    ${CMAKE_BINARY_DIR}/ev/api/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/algorithm/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/base/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/container/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/debugging/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/memory/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/meta/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/numeric/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/strings/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/synchronization/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/time/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/types/
    ${CMAKE_BINARY_DIR}/ev/external/abseil-cpp/absl/utility/
    ${CMAKE_BINARY_DIR}/ev/external/backward-cpp/
    ${CMAKE_BINARY_DIR}/ev/external/boringssl/
    ${CMAKE_BINARY_DIR}/ev/external/boringssl/crypto/
    ${CMAKE_BINARY_DIR}/ev/external/boringssl/decrepit/
    ${CMAKE_BINARY_DIR}/ev/external/boringssl/ssl/
    ${CMAKE_BINARY_DIR}/ev/external/cctz/
    ${CMAKE_BINARY_DIR}/ev/external/fmt-4.1.0/fmt/
    ${CMAKE_BINARY_DIR}/ev/external/googletest/googlemock/
    ${CMAKE_BINARY_DIR}/ev/external/googletest/googlemock/gtest/
    ${CMAKE_BINARY_DIR}/ev/external/grpc-httpjson-transcoding/
    ${CMAKE_BINARY_DIR}/ev/external/grpc/
    ${CMAKE_BINARY_DIR}/ev/external/grpc/third_party/benchmark/src/
    ${CMAKE_BINARY_DIR}/ev/external/grpc/third_party/benchmark/test/
    ${CMAKE_BINARY_DIR}/ev/external/grpc/third_party/cares/cares/lib/
    ${CMAKE_BINARY_DIR}/ev/external/grpc/third_party/gflags/
    ${CMAKE_BINARY_DIR}/ev/external/grpc/third_party/protobuf/
    ${CMAKE_BINARY_DIR}/ev/external/grpc/third_party/zlib/
    ${CMAKE_BINARY_DIR}/ev/external/http-parser/
    ${CMAKE_BINARY_DIR}/ev/external/jwt_verify_lib/
    ${CMAKE_BINARY_DIR}/ev/external/libprotobuf_mutator/src/
    ${CMAKE_BINARY_DIR}/ev/external/libprotobuf_mutator/src/libfuzzer/
    ${CMAKE_BINARY_DIR}/ev/external/lightstep-tracer-cpp/
    ${CMAKE_BINARY_DIR}/ev/external/opentracing-cpp/
    ${CMAKE_BINARY_DIR}/ev/external/opentracing-cpp/mocktracer/
    ${CMAKE_BINARY_DIR}/ev/external/yaml-cpp/
    ${CMAKE_BINARY_DIR}/ev/source/common/access_log/
    ${CMAKE_BINARY_DIR}/ev/source/common/api/
    ${CMAKE_BINARY_DIR}/ev/source/common/buffer/
    ${CMAKE_BINARY_DIR}/ev/source/common/common/
    ${CMAKE_BINARY_DIR}/ev/source/common/compressor/
    ${CMAKE_BINARY_DIR}/ev/source/common/config/
    ${CMAKE_BINARY_DIR}/ev/source/common/decompressor/
    ${CMAKE_BINARY_DIR}/ev/source/common/event/
    ${CMAKE_BINARY_DIR}/ev/source/common/filesystem/
    ${CMAKE_BINARY_DIR}/ev/source/common/grpc/
    ${CMAKE_BINARY_DIR}/ev/source/common/html/
    ${CMAKE_BINARY_DIR}/ev/source/common/http/
    ${CMAKE_BINARY_DIR}/ev/source/common/json/
    ${CMAKE_BINARY_DIR}/ev/source/common/memory/
    ${CMAKE_BINARY_DIR}/ev/source/common/network/
    ${CMAKE_BINARY_DIR}/ev/source/common/profiler/
    ${CMAKE_BINARY_DIR}/ev/source/common/protobuf/
    ${CMAKE_BINARY_DIR}/ev/source/common/ratelimit/
    ${CMAKE_BINARY_DIR}/ev/source/common/request_info/
    ${CMAKE_BINARY_DIR}/ev/source/common/router/
    ${CMAKE_BINARY_DIR}/ev/source/common/runtime/
    ${CMAKE_BINARY_DIR}/ev/source/common/secret/
    ${CMAKE_BINARY_DIR}/ev/source/common/singleton/
    ${CMAKE_BINARY_DIR}/ev/source/common/ssl/
    ${CMAKE_BINARY_DIR}/ev/source/common/stats/
    ${CMAKE_BINARY_DIR}/ev/source/common/tcp/
    ${CMAKE_BINARY_DIR}/ev/source/common/tcp_proxy/
    ${CMAKE_BINARY_DIR}/ev/source/common/thread_local/
    ${CMAKE_BINARY_DIR}/ev/source/common/tracing/
    ${CMAKE_BINARY_DIR}/ev/source/common/upstream/
    ${CMAKE_BINARY_DIR}/ev/source/exe/
    ${CMAKE_BINARY_DIR}/ev/source/extensions/
    ${CMAKE_BINARY_DIR}/ev/source/server/
    ${CMAKE_BINARY_DIR}/ev/source/server/config_validation/
    ${CMAKE_BINARY_DIR}/ev/source/server/http/
    ${CMAKE_BINARY_DIR}/ev/test/common/access_log/
    ${CMAKE_BINARY_DIR}/ev/test/common/api/
    ${CMAKE_BINARY_DIR}/ev/test/common/buffer/
    ${CMAKE_BINARY_DIR}/ev/test/common/common/
    ${CMAKE_BINARY_DIR}/ev/test/common/compressor/
    ${CMAKE_BINARY_DIR}/ev/test/common/config/
    ${CMAKE_BINARY_DIR}/ev/test/common/decompressor/
    ${CMAKE_BINARY_DIR}/ev/test/common/event/
    ${CMAKE_BINARY_DIR}/ev/test/common/filesystem/
    ${CMAKE_BINARY_DIR}/ev/test/common/grpc/
    ${CMAKE_BINARY_DIR}/ev/test/common/html/
    ${CMAKE_BINARY_DIR}/ev/test/common/http/
    ${CMAKE_BINARY_DIR}/ev/test/common/http/http1/
    ${CMAKE_BINARY_DIR}/ev/test/common/http/http2/
    ${CMAKE_BINARY_DIR}/ev/test/common/json/
    ${CMAKE_BINARY_DIR}/ev/test/common/network/
    ${CMAKE_BINARY_DIR}/ev/test/common/protobuf/
    ${CMAKE_BINARY_DIR}/ev/test/common/ratelimit/
    ${CMAKE_BINARY_DIR}/ev/test/common/request_info/
    ${CMAKE_BINARY_DIR}/ev/test/common/router/
    ${CMAKE_BINARY_DIR}/ev/test/common/runtime/
    ${CMAKE_BINARY_DIR}/ev/test/common/singleton/
    ${CMAKE_BINARY_DIR}/ev/test/common/ssl/
    ${CMAKE_BINARY_DIR}/ev/test/common/stats/
    ${CMAKE_BINARY_DIR}/ev/test/common/tcp_proxy/
    ${CMAKE_BINARY_DIR}/ev/test/common/thread_local/
    ${CMAKE_BINARY_DIR}/ev/test/common/tracing/
    ${CMAKE_BINARY_DIR}/ev/test/common/upstream/
    ${CMAKE_BINARY_DIR}/ev/test/config/
    ${CMAKE_BINARY_DIR}/ev/test/config_test/
    ${CMAKE_BINARY_DIR}/ev/test/coverage/gcc_only_test/
    ${CMAKE_BINARY_DIR}/ev/test/exe/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/access_loggers/file/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/access_loggers/http_grpc/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/common/ext_authz/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/common/lua/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/common/rbac/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/buffer/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/cors/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/dynamo/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/ext_authz/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/fault/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/grpc_http1_bridge/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/grpc_json_transcoder/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/grpc_web/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/gzip/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/health_check/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/ip_tagging/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/jwt_authn/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/lua/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/ratelimit/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/rbac/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/router/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/http/squash/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/listener/proxy_protocol/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/listener/tls_inspector/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/network/client_ssl_auth/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/network/ext_authz/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/network/http_connection_manager/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/network/mongo_proxy/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/network/ratelimit/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/network/redis_proxy/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/filters/network/tcp_proxy/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/health_checkers/redis/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/stats_sinks/common/statsd/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/stats_sinks/dog_statsd/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/stats_sinks/metrics_service/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/stats_sinks/statsd/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/tracers/common/ot/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/tracers/dynamic_ot/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/tracers/lightstep/
    ${CMAKE_BINARY_DIR}/ev/test/extensions/tracers/zipkin/
    ${CMAKE_BINARY_DIR}/ev/test/fuzz/
    ${CMAKE_BINARY_DIR}/ev/test/integration/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/access_log/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/api/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/buffer/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/config/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/event/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/filesystem/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/grpc/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/http/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/init/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/local_info/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/network/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/ratelimit/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/request_info/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/router/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/runtime/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/server/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/ssl/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/stats/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/thread_local/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/tracing/
    ${CMAKE_BINARY_DIR}/ev/test/mocks/upstream/
    ${CMAKE_BINARY_DIR}/ev/test/proto/
    ${CMAKE_BINARY_DIR}/ev/test/server/
    ${CMAKE_BINARY_DIR}/ev/test/server/config_validation/
    ${CMAKE_BINARY_DIR}/ev/test/server/http/
    ${CMAKE_BINARY_DIR}/ev/test/test_common/
    ${CMAKE_BINARY_DIR}/ev/test/tools/config_load_check/
    ${CMAKE_BINARY_DIR}/ev/test/tools/router_check/
    ${CMAKE_BINARY_DIR}/ev/test/tools/router_check/json/
    ${CMAKE_BINARY_DIR}/ev/test/tools/schema_validator/
    ${CMAKE_BINARY_DIR}/fs/app/libevent/lib/
    ${CMAKE_BINARY_DIR}/lib/fs/

    ${CMAKE_SOURCE_DIR}/ev/external/xxHash/
    ${CMAKE_SOURCE_DIR}/ev/external/luajit-2.0/src/
    ${CMAKE_SOURCE_DIR}/ev/external/libcircllhist/src/
    )

enable_testing()
add_executable(${project_name} ${sources})

add_custom_command(TARGET ${project_name}
    PRE_BUILD
    COMMAND echo " --- CUSTOM COMMAND ---"
    COMMAND touch ${CMAKE_SOURCE_DIR}/ev/test/main.cc
    )

set_target_properties(${project_name} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${project_name}
	-Wl,-whole-archive -lev-test-proto -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-test_common -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-server-http -Wl,-no-whole-archive
	ev-test-server
	-Wl,-whole-archive -lev-test-server-config_validation -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-config -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-extensions-access_loggers-file -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-extensions-access_loggers-http_grpc -Wl,-no-whole-archive
	# cannot find opentracing::v1::mocktracer::InMemoryRecorder::spans
	-Wl,-whole-archive -ltest-extensions-tracers-common-ot -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-extensions-tracers-lightstep -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-extensions-tracers-zipkin -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-extensions-stats_sinks-dog_statsd -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-extensions-stats_sinks-metrics_service -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-ratelimit -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-fault -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-lua -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-gzip -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-health_check -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-grpc_json_transcoder -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-rbac -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-ext_authz -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-grpc_web -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-router -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-cors -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-jwt_authn -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-buffer -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-dynamo -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-ip_tagging -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-grpc_http1_bridge -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-http-squash -Wl,-no-whole-archive
	# test-extensions-filters-listener-tls_inspector has main/exe of its own
	#-Wl,-whole-archive -ltest-extensions-filters-listener-tls_inspector -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-listener-proxy_protocol -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-common-lua -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-common-rbac -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-common-ext_authz -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-network-ratelimit -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-network-ext_authz -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-network-http_connection_manager -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-network-client_ssl_auth -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-network-mongo_proxy -Wl,-no-whole-archive
	-Wl,-whole-archive -ltest-extensions-filters-network-tcp_proxy -Wl,-no-whole-archive
	ev-test-extensions-stats_sinks-statsd
	test-extensions-stats_sinks-common-statsd
	test-extensions-filters-network-redis_proxy
	ev-test-extensions-health_checkers-redis
	ev-test-extensions-tracers-dynamic_ot
	ev-test-extensions
	# test-fuzz has a main/exe of its own
	#-Wl,-whole-archive -lev-test-fuzz -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-coverage-gcc_only_test -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-thread_local -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-ratelimit -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-grpc -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-stats -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-filesystem -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-config -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-runtime -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-request_info -Wl,-no-whole-archive
	# ev-test-common-access_log has main/exe of its own
	#-Wl,-whole-archive -lev-test-common-access_log -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-tracing -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-router -Wl,-no-whole-archive
	ev-test-common-http-http1
	ev-test-common-http-http2
	ev-test-common-http
	-Wl,-whole-archive -lev-test-common-buffer -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-html -Wl,-no-whole-archive
	# ev-test-common-common has a main/exe of its own
	#-Wl,-whole-archive -lev-test-common-common -Wl,-no-whole-archive
	# ev-test-common-network has a main/exe of its own
	#-Wl,-whole-archive -lev-test-common-network -Wl,-no-whole-archive
	ev-test-common-protobuf
	-Wl,-whole-archive -lev-test-common-singleton -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-event -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-compressor -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-ssl -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-decompressor -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-api -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-common-json -Wl,-no-whole-archive
	ev-test-common-tcp_proxy
	ev-test-common-upstream
	#ev-test-tools-schema_validator has a main of its own
	#-Wl,-whole-archive -lev-test-tools-schema_validator -Wl,-no-whole-archive
	#ev-test-tools-router_chech has a main/exe of its own
	#-Wl,-whole-archive -lev-test-tools-router_check -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-tools-router_check-json -Wl,-no-whole-archive
	ev-test-integration
	#ev-test-tools-config_load_check has main/exe of its own
	#-Wl,-whole-archive -lev-test-tools-config_load_check -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-config_test -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-exe -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-thread_local -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-http -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-ratelimit -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-grpc -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-stats -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-filesystem -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-server -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-config -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-runtime -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-upstream -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-request_info -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-access_log -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-tracing -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-local_info -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-router -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-init -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-buffer -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-network -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-event -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-ssl -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-test-mocks-api -Wl,-no-whole-archive
	ev-test-mocks
    ev-source-exe-lib
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
    ev-source-common-html
    ev-source-common-profiler
    -Wl,-whole-archive -lev-source-common-event -Wl,-no-whole-archive
    #network
    ev-source-common-json
	yaml-cpp
    ev-source-common-filesystem
    ev-source-common-api
    ev-source-common-router
    ev-source-common-grpc
    ev-source-common-stats
    -Wl,-whole-archive -lev-source-common-config -Wl,-no-whole-archive
    ev-source-server-http
	ev-test-test_common
    -Wl,-whole-archive -lcommon -Wl,-no-whole-archive
	-Wl,-whole-archive -lev-source-extensions  -Wl,-no-whole-archive
    ev-source-common-http
	ev-source-common-tcp_proxy
	ev-external-grpc-httpjson-transcoding
	ev-source-common-compressor
    -Wl,-whole-archive -lev-source-common-decompressor -Wl,-no-whole-archive
    -Wl,-whole-archive -lz -Wl,-no-whole-archive
    ev-source-common-access_log
    ev-external-http-parser
    ev-source-common-request_info
    ev-source-common-tracing
    -Wl,-whole-archive -llightstep_tracer -Wl,-no-whole-archive
    opentracing_mocktracer-static
	opentracing
    ev-source-common-runtime
    -Wl,-whole-archive -lev-source-common-network -Wl,-no-whole-archive
    nghttp2
    rt
    -Wl,-whole-archive -lev-source-common-protobuf -Wl,-no-whole-archive
    luajit
    -Wl,-whole-archive -lprotobuf -Wl,-no-whole-archive
	absl_utility
	absl_time
	#absl_memory
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
    -Wl,-whole-archive -lcares -Wl,-no-whole-archive
    ev-source-common-buffer
    pthread
    #event_pthreads
    event_core
    event
    event_extra
    gtest
    gmock
    ssl
    gpr
    grpc++_unsecure
    grpc
    grpc++
    ssl
    crypto
	stdc++fs
    sig_action
	circllhist
    protobuf-mutator-libfuzzer
    ev-source-common-secret
    ev-external-jwt_verify_lib
    fmt
)

#TODO: Add ev-test dependencies

add_dependencies(${project_name}
	ev-source-common-upstream
	ev-test-common-api
	ev-test-common-buffer
	ev-test-common-compressor
	ev-test-common-config
	ev-test-common-decompressor
	ev-test-common-event
	ev-test-common-filesystem
	ev-test-common-grpc
	ev-test-common-html
	ev-test-common-http-http1
	ev-test-common-json
	ev-test-common-protobuf
	ev-test-common-ratelimit
	ev-test-common-request_info
	ev-test-common-router
	ev-test-common-runtime
	ev-test-common-singleton
	ev-test-common-ssl
	ev-test-common-stats
	ev-test-common-thread_local
	ev-test-common-tracing
	ev-test-config
	ev-test-config_test
	ev-test-coverage-gcc_only_test
	ev-test-exe
	ev-test-extensions-access_loggers-file
	ev-test-extensions-access_loggers-http_grpc
	ev-test-extensions-stats_sinks-dog_statsd
	ev-test-extensions-stats_sinks-metrics_service
	ev-test-extensions-tracers-lightstep
	ev-test-extensions-tracers-zipkin
	ev-test-integration
	ev-test-mocks-access_log
	ev-test-mocks-api
	ev-test-mocks-buffer
	ev-test-mocks-config
	ev-test-mocks-event
	ev-test-mocks-filesystem
	ev-test-mocks-grpc
	ev-test-mocks-http
	ev-test-mocks-init
	ev-test-mocks-local_info
	ev-test-mocks-network
	ev-test-mocks-ratelimit
	ev-test-mocks-request_info
	ev-test-mocks-router
	ev-test-mocks-runtime
	ev-test-mocks-server
	ev-test-mocks-ssl
	ev-test-mocks-stats
	ev-test-mocks-thread_local
	ev-test-mocks-tracing
	ev-test-mocks-upstream
	ev-test-proto
	ev-test-server-config_validation
	ev-test-server-http
	ev-test-test_common
	ev-test-tools-router_check-json
	test-extensions-filters-common-ext_authz
	test-extensions-filters-common-lua
	test-extensions-filters-common-rbac
	test-extensions-filters-http-buffer
	test-extensions-filters-http-cors
	test-extensions-filters-http-dynamo
	test-extensions-filters-http-ext_authz
	test-extensions-filters-http-fault
	test-extensions-filters-http-grpc_http1_bridge
	test-extensions-filters-http-grpc_json_transcoder
	test-extensions-filters-http-grpc_web
	test-extensions-filters-http-gzip
	test-extensions-filters-http-health_check
	test-extensions-filters-http-ip_tagging
	test-extensions-filters-http-jwt_authn
	test-extensions-filters-http-lua
	test-extensions-filters-http-ratelimit
	test-extensions-filters-http-rbac
	test-extensions-filters-http-router
	test-extensions-filters-http-squash
	test-extensions-filters-listener-proxy_protocol
	test-extensions-filters-network-client_ssl_auth
	test-extensions-filters-network-ext_authz
	test-extensions-filters-network-http_connection_manager
	test-extensions-filters-network-mongo_proxy
	test-extensions-filters-network-ratelimit
	test-extensions-filters-network-tcp_proxy
	test-extensions-tracers-common-ot
	ev-external-grpc-httpjson-transcoding
	ev-source-common-compressor
	ev-source-common-ratelimit
	ev-source-common-singleton
	ev-source-common-ssl
	ev-source-common-tcp_proxy
	ev-source-common-thread_local
	ev-source-extensions
	ev-test-common-http
	ev-test-common-http-http2
	ev-test-common-tcp_proxy
	ev-test-common-upstream
	ev-test-extensions
	ev-test-extensions-health_checkers-redis
	ev-test-extensions-stats_sinks-statsd
	ev-test-extensions-tracers-dynamic_ot
	ev-test-mocks
	ev-test-server
	ev-test-test_common
	test-extensions-filters-network-redis_proxy
	test-extensions-stats_sinks-common-statsd
	ev-source-common-config
	ev-source-common-decompressor
	ev-source-common-event
	ev-source-common-network
	ev-source-common-protobuf
	ev-source-server
	protos-api
	ev-external-http-parser
	ev-source-common-access_log
	ev-source-common-api
	ev-source-common-buffer
	ev-source-common-filesystem
	ev-source-common-grpc
	ev-source-common-html
	ev-source-common-http
	ev-source-common-json
	ev-source-common-memory
	ev-source-common-profiler
	ev-source-common-request_info
	ev-source-common-router
	ev-source-common-runtime
	ev-source-common-stats
	ev-source-common-tracing
	ev-source-server-http
)
