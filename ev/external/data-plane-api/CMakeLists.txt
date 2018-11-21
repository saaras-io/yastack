include(ExternalProject)


get_filename_component(project_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${project_name})

set(ev_external_dir             ${CMAKE_SOURCE_DIR}/ev/external)
set(ev_dir                      ${CMAKE_SOURCE_DIR}/ev/)

file(GLOB_RECURSE
    proto_sources
    ${ev_dir}/api/*.proto
    ${ev_external_dir}/grpc/*.proto
    ${ev_external_dir}/googleapis/google/api/*.proto
    ${ev_external_dir}/protoc-gen-validate/validate/*.proto
    ${ev_external_dir}/gogo-proto/gogoproto/*.proto
    ${ev_external_dir}/lightstep-tracer-cpp/lightstep-tracer-common/*.proto
    ${ev_external_dir}/opencensus-proto/opencensus/proto/trace/*.proto
    ${ev_external_dir}/client_model/*.proto
    ${ev_dir}/test/integration/*.proto
    ${ev_dir}/source/common/ratelimit/*.proto
)

set(gen_script                  ${ev_external_dir}/data-plane-api/generate_pb.py)
set(gen_protos_incl             ${CMAKE_BINARY_DIR}/protos)
set(gen_output                  ${CMAKE_BINARY_DIR}/include)

#set(grpc_health_proto_path      ${gen_protos_incl}/src/proto/grpc/health/v1/)
set(grpc_health_proto_path      ${gen_protos_incl}/google.golang.org/grpc/health/grpc_health_v1/)
set(grpc_status_proto_path      ${gen_protos_incl}/google/rpc/)
set(protocgen_validate_path     ${gen_protos_incl}/validate/)
set(gogo_proto_path             ${gen_protos_incl}/gogoproto)
set(google_api                  ${gen_protos_incl}/google/api/)
set(lightstep_tracer            ${gen_protos_incl}/lightstep-tracer-common/)
set(test_integration            ${gen_protos_incl}/test/integration/)
set(source_common_ratelimit     ${gen_protos_incl}/source/common/ratelimit)
set(source_validate				${gen_protos_incl}/validate)
#set(incl_test_proto				${gen_protos_incl}/test/proto)
set(ev_protos_project           ev_protos-${project_name})
set(rsync_cmd                   rsync -auv)
set(bin_include                 ${CMAKE_BINARY_DIR}/include)

set(fuzzer_proto_path3          ${gen_protos_incl}/test/common/access_log)
set(fuzzer_proto_path           ${gen_protos_incl}/test/common/http)
set(fuzzer_proto_path4          ${gen_protos_incl}/test/common/http2)
set(fuzzer_proto_path5          ${gen_protos_incl}/test/common/router)
set(fuzzer_proto_path2          ${gen_protos_incl}/test/fuzz)

### High level target ###

add_custom_target(${project_name}
    ALL
    DEPENDS ${ev_protos_project}
    DEPENDS ${proto_sources}
    )

### copy files ###

add_custom_target(target_copy
    # invoke the custom command that has this OUTPUT
    DEPENDS ${proto_sources}
    )

#add_custom_command(TARGET target_copy
#    COMMAND rm -rf ${gen_protos_incl}/metrics.proto
#    COMMAND rm -rf ${gen_output}/metrics.pb.h
#    PRE_BUILD
#    )

#Copy protos from dependencies which aren't automatically copied.
#All the protos under the API rectory are automatically copied.
#Remaining ones from external and other locations are cherry picked
add_custom_command(TARGET target_copy

    DEPENDS ${proto_sources}

    COMMAND mkdir -p ${gen_protos_incl}
    COMMAND mkdir -p ${gen_output}
    COMMAND ${rsync_cmd} ${ev_dir}/api/ ${gen_protos_incl}

    COMMAND mkdir -p ${grpc_health_proto_path}
    COMMAND find ${ev_external_dir}/grpc/ -name health.proto -exec ${rsync_cmd} {} ${grpc_health_proto_path} '\;'

    COMMAND mkdir -p ${grpc_status_proto_path}
    COMMAND find ${ev_external_dir}/grpc/src/proto -name status.proto -exec ${rsync_cmd} {} ${grpc_status_proto_path} '\;'

    COMMAND mkdir -p ${protocgen_validate_path}
    COMMAND find ${ev_external_dir}/protoc-gen-validate/validate -name validate.proto -exec ${rsync_cmd} {} ${protocgen_validate_path} '\;'

    COMMAND mkdir -p ${gogo_proto_path}
    COMMAND find ${ev_external_dir}/gogo-proto/gogoproto/ -name gogo.proto -exec ${rsync_cmd} {} ${gogo_proto_path} '\;'

    COMMAND mkdir -p ${google_api}
    COMMAND find ${ev_external_dir}/lightstep-tracer-cpp/lightstep-tracer-common/third_party/googleapis/google/api/ -name annotations.proto -exec ${rsync_cmd} {} ${google_api} '\;'
    COMMAND find ${ev_external_dir}/googleapis/google/api/ -name httpbody.proto -exec ${rsync_cmd} {} ${google_api} '\;'

    COMMAND find ${ev_external_dir}/lightstep-tracer-cpp/lightstep-tracer-common/third_party/googleapis/google/api/ -name http.proto -exec ${rsync_cmd} {} ${google_api} '\;'
    COMMAND find ${ev_external_dir}/opencensus-proto/opencensus/proto/trace/ -name trace.proto -exec ${rsync_cmd} {} ${gen_protos_incl} '\;'
    COMMAND find ${ev_external_dir}/client_model/ -name metrics.proto -exec ${rsync_cmd} {} ${gen_protos_incl} '\;'

    COMMAND mkdir -p ${lightstep_tracer}
    COMMAND find ${ev_external_dir}/lightstep-tracer-cpp/lightstep-tracer-common/ -name collector.proto -exec ${rsync_cmd} {} ${lightstep_tracer} '\;'
    COMMAND find ${ev_external_dir}/lightstep-tracer-cpp/lightstep-tracer-common/ -name lightstep_carrier.proto -exec ${rsync_cmd} {} ${lightstep_tracer} '\;'

    COMMAND mkdir -p ${test_integration}
    COMMAND find ${ev_dir}/test/integration -name capture_fuzz.proto -exec ${rsync_cmd} {} ${test_integration} '\;'

    COMMAND mkdir -p ${source_common_ratelimit}
    COMMAND find ${ev_dir}/source/common/ratelimit -name ratelimit.proto -exec ${rsync_cmd} {} ${source_common_ratelimit} '\;'

	COMMAND mkdir -p ${source_validate}
    COMMAND find ${ev_dir}/external/protoc-gen-validate -name validate.proto -exec ${rsync_cmd} {} ${source_validate} '\;'

	COMMAND mkdir -p ${fuzzer_proto_path}
    COMMAND find ${ev_dir}/test/common/http/ -name header_map_impl_fuzz.proto -exec ${rsync_cmd} {} ${fuzzer_proto_path} '\;'
    COMMAND find ${ev_dir}/test/common/http/ -name conn_manager_impl_fuzz.proto -exec ${rsync_cmd} {} ${fuzzer_proto_path} '\;'
    COMMAND find ${ev_dir}/test/common/http/ -name utility_fuzz.proto -exec ${rsync_cmd} {} ${fuzzer_proto_path} '\;'

	COMMAND mkdir -p ${fuzzer_proto_path2}
    COMMAND find ${ev_dir}/test/fuzz/ -name common.proto -exec ${rsync_cmd} {} ${fuzzer_proto_path2} '\;'

	COMMAND mkdir -p ${fuzzer_proto_path3}
    COMMAND find ${ev_dir}/test/common/access_log/ -name access_log_formatter_fuzz.proto -exec ${rsync_cmd} {} ${fuzzer_proto_path3} '\;'

	COMMAND mkdir -p ${fuzzer_proto_path4}
    COMMAND find ${ev_dir}/test/common/http/http2/ -name codec_impl_fuzz.proto -exec ${rsync_cmd} {} ${fuzzer_proto_path4} '\;'

	COMMAND mkdir -p ${fuzzer_proto_path5}
    COMMAND find ${ev_dir}/test/common/router/ -name header_parser_fuzz.proto -exec ${rsync_cmd} {} ${fuzzer_proto_path5} '\;'
    COMMAND find ${ev_dir}/test/common/router/ -name route_fuzz.proto -exec ${rsync_cmd} {} ${fuzzer_proto_path5} '\;'

    #	COMMAND mkdir -p ${incl_test_proto}
    #    COMMAND find ${ev_dir}/test/proto/ -name bookstore.proto -exec ${rsync_cmd} {} ${incl_test_proto} '\;'
    #    COMMAND find ${ev_dir}/test/proto/ -name helloworld.proto -exec ${rsync_cmd} {} ${incl_test_proto} '\;'

    )

### generate files ###

add_custom_target(target_gen
    DEPENDS ${gen_output}/metrics.pb.h
    )

find_program(py python)

add_custom_command(OUTPUT ${gen_output}/metrics.pb.h
    DEPENDS target_copy
    COMMAND ${py} ${gen_script} --generate --include='-I ${gen_protos_incl}' --output=${gen_output} --recurse-start-dir=${gen_protos_incl}

	# purge un-necessary file.
	# the protoc-gen-validate generates validation rules for ALL protobufs.
	# the validation rules are only relevant for user-defined-protos, not for others.
	# purge the un-necessary protobufs here
	COMMAND find ${CMAKE_BINARY_DIR}/include -name annotations.pb.validate.h -exec rm -rf {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/include -name annotations.pb.validate.cc -exec rm -rf {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/include -name http.pb.validate.cc -exec rm -rf {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/include -name http.pb.validate.h -exec rm -rf {} '\;'

	COMMAND find ${CMAKE_BINARY_DIR}/include -name collector.pb.validate.h -exec rm -rf {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/include -name lightstep_carrier.pb.validate.h -exec rm -rf {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/include -name collector.pb.validate.cc -exec rm -rf {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/include -name lightstep_carrier.pb.validate.cc -exec rm -rf {} '\;'

	COMMAND find ${CMAKE_BINARY_DIR}/include -name status.pb.validate.cc -exec rm -rf {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/include -name status.pb.validate.h -exec rm -rf {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/include -name capture_fuzz.pb.validate.h -exec rm -rf {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/include -name capture_fuzz.pb.validate.cc -exec rm -rf {} '\;'

	COMMAND find ${CMAKE_BINARY_DIR}/include -name validate.pb.validate.h -exec rm -rf {} '\;'
	COMMAND find ${CMAKE_BINARY_DIR}/include -name validate.pb.validate.cc -exec rm -rf {} '\;'

    DEPENDS ${proto_sources}
    )

### build lib ###

file(GLOB_RECURSE
    proto_cc_h
    ${CMAKE_BINARY_DIR}/include/*.cc
    ${CMAKE_BINARY_DIR}/include/*.h
)

set(proto_cc_h_list
    #	${bin_include}/metrics.pb.h
    #	${bin_include}/src/proto/grpc/health/v1/health.pb.h
    #	${bin_include}/gogoproto/gogo.pb.h
    #	${bin_include}/trace.pb.h
    #	${bin_include}/envoy/config/ratelimit/v2/rls.pb.h
    #	${bin_include}/envoy/config/accesslog/v2/als.pb.h
    #	${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.h
    #	${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.h
    #	${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.h
    #	${bin_include}/envoy/config/trace/v2/trace.pb.h
    #	${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.h
    #	${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.h
    #	${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.h
    #	${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.h
    #	${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.h
    #	${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.h
    #	${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.h
    #	${bin_include}/envoy/config/filter/http/router/v2/router.pb.h
    #	${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.h
    #	${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.h
    #	${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.h
    #	${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.h
    #	${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.h
    #	${bin_include}/envoy/config/filter/fault/v2/fault.pb.h
    #	${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.h
    #	${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.h
    #	${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.h
    #	${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.h
    #	${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.h
    #	${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.h
    #	${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.h
    #	${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.h
    #	${bin_include}/envoy/config/metrics/v2/stats.pb.h
    #	${bin_include}/envoy/config/metrics/v2/metrics_service.pb.h
    #	${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.h
    #	${bin_include}/envoy/extensions/common/tap/v2alpha/capture.pb.h
    #	${bin_include}/envoy/admin/v2alpha/config_dump.pb.h
    #	${bin_include}/envoy/service/ratelimit/v2/rls.pb.h
    #	${bin_include}/envoy/service/load_stats/v2/lrs.pb.h
    #	${bin_include}/envoy/service/discovery/v2/sds.pb.h
    #	${bin_include}/envoy/service/discovery/v2/hds.pb.h
    #	${bin_include}/envoy/service/discovery/v2/ads.pb.h
    #	${bin_include}/envoy/service/accesslog/v2/als.pb.h
    #	${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.h
    #	${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.h
    #	${bin_include}/envoy/service/trace/v2/trace_service.pb.h
    #	${bin_include}/envoy/service/metrics/v2/metrics_service.pb.h
    #	${bin_include}/envoy/type/percent.pb.h
    #	${bin_include}/envoy/type/range.pb.h
    #	${bin_include}/envoy/type/string_match.pb.h
    #	${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.h
    #	${bin_include}/envoy/api/v2/cds.pb.h
    #	${bin_include}/envoy/api/v2/listener/listener.pb.h
    #	${bin_include}/envoy/api/v2/auth/cert.pb.h
    #	${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.h
    #	${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.h
    #	${bin_include}/envoy/api/v2/endpoint/endpoint.pb.h
    #	${bin_include}/envoy/api/v2/endpoint/load_report.pb.h
    #	${bin_include}/envoy/api/v2/eds.pb.h
    #	${bin_include}/envoy/api/v2/route/route.pb.h
    #	${bin_include}/envoy/api/v2/rds.pb.h
    #	${bin_include}/envoy/api/v2/core/grpc_service.pb.h
    #	${bin_include}/envoy/api/v2/core/config_source.pb.h
    #	${bin_include}/envoy/api/v2/core/base.pb.h
    #	${bin_include}/envoy/api/v2/core/health_check.pb.h
    #	${bin_include}/envoy/api/v2/core/address.pb.h
    #	${bin_include}/envoy/api/v2/core/protocol.pb.h
    #	${bin_include}/envoy/api/v2/core/http_uri.pb.h
    #	${bin_include}/envoy/api/v2/lds.pb.h
    #	${bin_include}/envoy/api/v2/discovery.pb.h
    #	${bin_include}/lightstep-tracer-common/lightstep_carrier.pb.h
    #	${bin_include}/lightstep-tracer-common/collector.pb.h
    #	${bin_include}/validate/validate.pb.h
    #	${bin_include}/google/api/http.pb.h
    #	${bin_include}/google/api/annotations.pb.h
    #	${bin_include}/google/rpc/status.pb.h
    #	${bin_include}/metrics.pb.cc
    #	${bin_include}/src/proto/grpc/health/v1/health.pb.cc
    #	${bin_include}/gogoproto/gogo.pb.cc
    #	${bin_include}/envoy/config/ratelimit/v2/rls.pb.cc
    #	${bin_include}/envoy/config/accesslog/v2/als.pb.cc
    #	${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.cc
    #	${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.cc
    #	${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.cc
    #	${bin_include}/envoy/config/trace/v2/trace.pb.cc
    #	${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.cc
    #	${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.cc
    #	${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.cc
    #	${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.cc
    #	${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.cc
    #	${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.cc
    #	${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.cc
    #	${bin_include}/envoy/config/filter/http/router/v2/router.pb.cc
    #	${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.cc
    #	${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.cc
    #	${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.cc
    #	${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.cc
    #	${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.cc
    #	${bin_include}/envoy/config/filter/fault/v2/fault.pb.cc
    #	${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.cc
    #	${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.cc
    #	${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.cc
    #	${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.cc
    #	${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.cc
    #	${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.cc
    #	${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.cc
    #	${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.cc
    #	${bin_include}/envoy/config/metrics/v2/stats.pb.cc
    #	${bin_include}/envoy/config/metrics/v2/metrics_service.pb.cc
    #	${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.cc
    #	${bin_include}/envoy/extensions/common/tap/v2alpha/capture.pb.cc
    #	${bin_include}/envoy/admin/v2alpha/config_dump.pb.cc
    #	${bin_include}/envoy/service/ratelimit/v2/rls.pb.cc
    #	${bin_include}/envoy/service/load_stats/v2/lrs.pb.cc
    #	${bin_include}/envoy/service/discovery/v2/hds.pb.cc
    #	${bin_include}/envoy/service/discovery/v2/sds.pb.cc
    #	${bin_include}/envoy/service/discovery/v2/ads.pb.cc
    #	${bin_include}/envoy/service/accesslog/v2/als.pb.cc
    #	${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.cc
    #	${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.cc
    #	${bin_include}/envoy/service/trace/v2/trace_service.pb.cc
    #	${bin_include}/envoy/service/metrics/v2/metrics_service.pb.cc
    #	${bin_include}/envoy/type/range.pb.cc
    #	${bin_include}/envoy/type/string_match.pb.cc
    #	${bin_include}/envoy/type/percent.pb.cc
    #	${bin_include}/envoy/api/v2/eds.pb.cc
    #	${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.cc
    #	${bin_include}/envoy/api/v2/rds.pb.cc
    #	${bin_include}/envoy/api/v2/discovery.pb.cc
    #	${bin_include}/envoy/api/v2/cds.pb.cc
    #	${bin_include}/envoy/api/v2/listener/listener.pb.cc
    #	${bin_include}/envoy/api/v2/auth/cert.pb.cc
    #	${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.cc
    #	${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.cc
    #	${bin_include}/envoy/api/v2/endpoint/endpoint.pb.cc
    #	${bin_include}/envoy/api/v2/endpoint/load_report.pb.cc
    #	${bin_include}/envoy/api/v2/route/route.pb.cc
    #	${bin_include}/envoy/api/v2/core/grpc_service.pb.cc
    #	${bin_include}/envoy/api/v2/core/http_uri.pb.cc
    #	${bin_include}/envoy/api/v2/core/protocol.pb.cc
    #	${bin_include}/envoy/api/v2/core/address.pb.cc
    #	${bin_include}/envoy/api/v2/core/config_source.pb.cc
    #	${bin_include}/envoy/api/v2/core/base.pb.cc
    #	${bin_include}/envoy/api/v2/core/health_check.pb.cc
    #	${bin_include}/envoy/api/v2/lds.pb.cc
    #	${bin_include}/lightstep-tracer-common/collector.pb.cc
    #	${bin_include}/lightstep-tracer-common/lightstep_carrier.pb.cc
    #	${bin_include}/validate/validate.pb.cc
    #	${bin_include}/trace.pb.cc
    #	${bin_include}/google/api/annotations.pb.cc
    #	${bin_include}/google/api/http.pb.cc
    #	${bin_include}/google/rpc/status.pb.cc
    #	${bin_include}/test/integration/capture_fuzz.pb.h
    #	${bin_include}/test/integration/capture_fuzz.pb.cc
    #	${bin_include}/source/common/ratelimit/ratelimit.pb.h
    #	${bin_include}/source/common/ratelimit/ratelimit.pb.cc
    #	# validate protos
    #	${bin_include}/trace.pb.validate.cc
    #	${bin_include}/src/proto/grpc/health/v1/health.pb.validate.h
    #	${bin_include}/src/proto/grpc/health/v1/health.pb.validate.cc
    #	${bin_include}/source/common/ratelimit/ratelimit.pb.validate.h
    #	${bin_include}/source/common/ratelimit/ratelimit.pb.validate.cc
    #	#${bin_include}/github.com/gogo/protobuf/gogoproto/gogo.pb.validate.cc
    #	#${bin_include}/github.com/gogo/protobuf/gogoproto/gogo.pb.validate.h
    #	${bin_include}/envoy/config/ratelimit/v2/rls.pb.validate.cc
    #	${bin_include}/envoy/config/ratelimit/v2/rls.pb.validate.h
    #	${bin_include}/envoy/config/accesslog/v2/als.pb.validate.h
    #	${bin_include}/envoy/config/accesslog/v2/als.pb.validate.cc
    #	${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.validate.h
    #	${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.validate.cc
    #	${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.validate.cc
    #	${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.validate.h
    #	${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.validate.h
    #	${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.validate.cc
    #	${bin_include}/envoy/config/trace/v2/trace.pb.validate.cc
    #	${bin_include}/envoy/config/trace/v2/trace.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/router/v2/router.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/router/v2/router.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.validate.h
    #	${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.validate.cc
    #	${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.validate.h
    #	${bin_include}/envoy/config/filter/fault/v2/fault.pb.validate.cc
    #	${bin_include}/envoy/config/filter/fault/v2/fault.pb.validate.h
    #	${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.validate.cc
    #	${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.validate.h
    #	${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.validate.cc
    #	${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.validate.h
    #	${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.validate.h
    #	${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.validate.cc
    #	${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.validate.cc
    #	${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.validate.h
    #	${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.validate.cc
    #	${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.validate.h
    #	${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.validate.cc
    #	${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.validate.h
    #	${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.validate.cc
    #	${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.validate.h
    #	${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.validate.h
    #	${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.validate.cc
    #	${bin_include}/envoy/config/metrics/v2/metrics_service.pb.validate.h
    #	${bin_include}/envoy/config/metrics/v2/metrics_service.pb.validate.cc
    #	${bin_include}/envoy/config/metrics/v2/stats.pb.validate.cc
    #	${bin_include}/envoy/config/metrics/v2/stats.pb.validate.h
    #	${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.validate.h
    #	${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.validate.cc
    #	${bin_include}/envoy/extensions/common/tap/v2alpha/capture.pb.validate.h
    #	${bin_include}/envoy/extensions/common/tap/v2alpha/capture.pb.validate.cc
    #	${bin_include}/envoy/admin/v2alpha/config_dump.pb.validate.h
    #	${bin_include}/envoy/admin/v2alpha/config_dump.pb.validate.cc
    #	${bin_include}/envoy/service/ratelimit/v2/rls.pb.validate.cc
    #	${bin_include}/envoy/service/ratelimit/v2/rls.pb.validate.h
    #	${bin_include}/envoy/service/load_stats/v2/lrs.pb.validate.h
    #	${bin_include}/envoy/service/load_stats/v2/lrs.pb.validate.cc
    #	${bin_include}/envoy/service/discovery/v2/hds.pb.validate.cc
    #	${bin_include}/envoy/service/discovery/v2/ads.pb.validate.h
    #	${bin_include}/envoy/service/discovery/v2/sds.pb.validate.h
    #	${bin_include}/envoy/service/discovery/v2/sds.pb.validate.cc
    #	${bin_include}/envoy/service/discovery/v2/hds.pb.validate.h
    #	${bin_include}/envoy/service/discovery/v2/ads.pb.validate.cc
    #	${bin_include}/envoy/service/accesslog/v2/als.pb.validate.h
    #	${bin_include}/envoy/service/accesslog/v2/als.pb.validate.cc
    #	${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.validate.h
    #	${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.validate.cc
    #	${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.validate.h
    #	${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.validate.cc
    #	${bin_include}/envoy/service/trace/v2/trace_service.pb.validate.h
    #	${bin_include}/envoy/service/trace/v2/trace_service.pb.validate.cc
    #	${bin_include}/envoy/service/metrics/v2/metrics_service.pb.validate.h
    #	${bin_include}/envoy/service/metrics/v2/metrics_service.pb.validate.cc
    #	${bin_include}/envoy/type/percent.pb.validate.h
    #	${bin_include}/envoy/type/percent.pb.validate.cc
    #	${bin_include}/envoy/type/string_match.pb.validate.h
    #	${bin_include}/envoy/type/string_match.pb.validate.cc
    #	${bin_include}/envoy/type/range.pb.validate.cc
    #	${bin_include}/envoy/type/range.pb.validate.h
    #	${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.validate.h
    #	${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.validate.cc
    #	${bin_include}/envoy/api/v2/eds.pb.validate.cc
    #	${bin_include}/envoy/api/v2/discovery.pb.validate.cc
    #	${bin_include}/envoy/api/v2/eds.pb.validate.h
    #	${bin_include}/envoy/api/v2/lds.pb.validate.cc
    #	${bin_include}/envoy/api/v2/listener/listener.pb.validate.h
    #	${bin_include}/envoy/api/v2/listener/listener.pb.validate.cc
    #	${bin_include}/envoy/api/v2/auth/cert.pb.validate.h
    #	${bin_include}/envoy/api/v2/auth/cert.pb.validate.cc
    #	${bin_include}/envoy/api/v2/discovery.pb.validate.h
    #	${bin_include}/envoy/api/v2/lds.pb.validate.h
    #	${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.validate.h
    #	${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.validate.h
    #	${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.validate.cc
    #	${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.validate.cc
    #	${bin_include}/envoy/api/v2/endpoint/endpoint.pb.validate.cc
    #	${bin_include}/envoy/api/v2/endpoint/load_report.pb.validate.cc
    #	${bin_include}/envoy/api/v2/endpoint/load_report.pb.validate.h
    #	${bin_include}/envoy/api/v2/endpoint/endpoint.pb.validate.h
    #	${bin_include}/envoy/api/v2/cds.pb.validate.cc
    #	${bin_include}/envoy/api/v2/route/route.pb.validate.h
    #	${bin_include}/envoy/api/v2/route/route.pb.validate.cc
    #	${bin_include}/envoy/api/v2/core/health_check.pb.validate.h
    #	${bin_include}/envoy/api/v2/core/protocol.pb.validate.cc
    #	${bin_include}/envoy/api/v2/core/protocol.pb.validate.h
    #	${bin_include}/envoy/api/v2/core/base.pb.validate.h
    #	${bin_include}/envoy/api/v2/core/address.pb.validate.cc
    #	${bin_include}/envoy/api/v2/core/config_source.pb.validate.h
    #	${bin_include}/envoy/api/v2/core/base.pb.validate.cc
    #	${bin_include}/envoy/api/v2/core/address.pb.validate.h
    #	${bin_include}/envoy/api/v2/core/grpc_service.pb.validate.cc
    #	${bin_include}/envoy/api/v2/core/grpc_service.pb.validate.h
    #	${bin_include}/envoy/api/v2/core/config_source.pb.validate.cc
    #	${bin_include}/envoy/api/v2/core/health_check.pb.validate.cc
    #	${bin_include}/envoy/api/v2/core/http_uri.pb.validate.h
    #	${bin_include}/envoy/api/v2/core/http_uri.pb.validate.cc
    #	${bin_include}/envoy/api/v2/rds.pb.validate.cc
    #	${bin_include}/envoy/api/v2/cds.pb.validate.h
    #	${bin_include}/envoy/api/v2/rds.pb.validate.h
    #	${bin_include}/metrics.pb.validate.h
    #	${bin_include}/metrics.pb.validate.cc
    #	${bin_include}/trace.pb.validate.h

    # commenting out on 6/22/2018, adding a new sorted one after comparison.
    #        ${bin_include}/envoy/admin/v2alpha/config_dump.pb.cc
    #        ${bin_include}/envoy/admin/v2alpha/config_dump.pb.h
    #        ${bin_include}/envoy/admin/v2alpha/config_dump.pb.validate.cc
    #        ${bin_include}/envoy/admin/v2alpha/config_dump.pb.validate.h
    #        ${bin_include}/envoy/admin/v2alpha/clusters.pb.cc
    #        ${bin_include}/envoy/admin/v2alpha/clusters.pb.h
    #        ${bin_include}/envoy/admin/v2alpha/metrics.pb.cc
    #        ${bin_include}/envoy/admin/v2alpha/metrics.pb.h
    #        ${bin_include}/envoy/api/v2/auth/cert.pb.cc
    #        ${bin_include}/envoy/api/v2/auth/cert.pb.h
    #        ${bin_include}/envoy/api/v2/auth/cert.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/auth/cert.pb.validate.h
    #        ${bin_include}/envoy/api/v2/cds.pb.cc
    #        ${bin_include}/envoy/api/v2/cds.pb.h
    #        ${bin_include}/envoy/api/v2/cds.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/cds.pb.validate.h
    #        ${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.cc
    #        ${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.h
    #        ${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.validate.h
    #        ${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.cc
    #        ${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.h
    #        ${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.validate.h
    #        ${bin_include}/envoy/api/v2/core/address.pb.cc
    #        ${bin_include}/envoy/api/v2/core/address.pb.h
    #        ${bin_include}/envoy/api/v2/core/address.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/core/address.pb.validate.h
    #        ${bin_include}/envoy/api/v2/core/base.pb.cc
    #        ${bin_include}/envoy/api/v2/core/base.pb.h
    #        ${bin_include}/envoy/api/v2/core/base.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/core/base.pb.validate.h
    #        ${bin_include}/envoy/api/v2/core/config_source.pb.cc
    #        ${bin_include}/envoy/api/v2/core/config_source.pb.h
    #        ${bin_include}/envoy/api/v2/core/config_source.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/core/config_source.pb.validate.h
    #        ${bin_include}/envoy/api/v2/core/grpc_service.pb.cc
    #        ${bin_include}/envoy/api/v2/core/grpc_service.pb.h
    #        ${bin_include}/envoy/api/v2/core/grpc_service.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/core/grpc_service.pb.validate.h
    #        ${bin_include}/envoy/api/v2/core/health_check.pb.cc
    #        ${bin_include}/envoy/api/v2/core/health_check.pb.h
    #        ${bin_include}/envoy/api/v2/core/health_check.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/core/health_check.pb.validate.h
    #        ${bin_include}/envoy/api/v2/core/http_uri.pb.cc
    #        ${bin_include}/envoy/api/v2/core/http_uri.pb.h
    #        ${bin_include}/envoy/api/v2/core/http_uri.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/core/http_uri.pb.validate.h
    #        ${bin_include}/envoy/api/v2/core/protocol.pb.cc
    #        ${bin_include}/envoy/api/v2/core/protocol.pb.h
    #        ${bin_include}/envoy/api/v2/core/protocol.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/core/protocol.pb.validate.h
    #        ${bin_include}/envoy/api/v2/discovery.pb.cc
    #        ${bin_include}/envoy/api/v2/discovery.pb.h
    #        ${bin_include}/envoy/api/v2/discovery.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/discovery.pb.validate.h
    #        ${bin_include}/envoy/api/v2/eds.pb.cc
    #        ${bin_include}/envoy/api/v2/eds.pb.h
    #        ${bin_include}/envoy/api/v2/eds.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/eds.pb.validate.h
    #        ${bin_include}/envoy/api/v2/endpoint/endpoint.pb.cc
    #        ${bin_include}/envoy/api/v2/endpoint/endpoint.pb.h
    #        ${bin_include}/envoy/api/v2/endpoint/endpoint.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/endpoint/endpoint.pb.validate.h
    #        ${bin_include}/envoy/api/v2/endpoint/load_report.pb.cc
    #        ${bin_include}/envoy/api/v2/endpoint/load_report.pb.h
    #        ${bin_include}/envoy/api/v2/endpoint/load_report.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/endpoint/load_report.pb.validate.h
    #        ${bin_include}/envoy/api/v2/lds.pb.cc
    #        ${bin_include}/envoy/api/v2/lds.pb.h
    #        ${bin_include}/envoy/api/v2/lds.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/lds.pb.validate.h
    #        ${bin_include}/envoy/api/v2/listener/listener.pb.cc
    #        ${bin_include}/envoy/api/v2/listener/listener.pb.h
    #        ${bin_include}/envoy/api/v2/listener/listener.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/listener/listener.pb.validate.h
    #        ${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.cc
    #        ${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.h
    #        ${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.validate.h
    #        ${bin_include}/envoy/api/v2/rds.pb.cc
    #        ${bin_include}/envoy/api/v2/rds.pb.h
    #        ${bin_include}/envoy/api/v2/rds.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/rds.pb.validate.h
    #        ${bin_include}/envoy/api/v2/route/route.pb.cc
    #        ${bin_include}/envoy/api/v2/route/route.pb.h
    #        ${bin_include}/envoy/api/v2/route/route.pb.validate.cc
    #        ${bin_include}/envoy/api/v2/route/route.pb.validate.h
    #        ${bin_include}/envoy/config/accesslog/v2/als.pb.cc
    #        ${bin_include}/envoy/config/accesslog/v2/als.pb.h
    #        ${bin_include}/envoy/config/accesslog/v2/als.pb.validate.cc
    #        ${bin_include}/envoy/config/accesslog/v2/als.pb.validate.h
    #        ${bin_include}/envoy/config/accesslog/v2/file.pb.cc
    #        ${bin_include}/envoy/config/accesslog/v2/file.pb.h
    #        ${bin_include}/envoy/config/accesslog/v2/file.pb.validate.cc
    #        ${bin_include}/envoy/config/accesslog/v2/file.pb.validate.h
    #        ${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.cc
    #        ${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.h
    #        ${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.validate.cc
    #        ${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.validate.h
    #        ${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.cc
    #        ${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.h
    #        ${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.validate.h
    #        ${bin_include}/envoy/config/filter/fault/v2/fault.pb.cc
    #        ${bin_include}/envoy/config/filter/fault/v2/fault.pb.h
    #        ${bin_include}/envoy/config/filter/fault/v2/fault.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/fault/v2/fault.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.cc
    #        ${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.h
    #        ${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.cc
    #        ${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.h
    #        ${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.cc
    #        ${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.h
    #        ${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.cc
    #        ${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.h
    #        ${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.cc
    #        ${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.h
    #        ${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.cc
    #        ${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.h
    #        ${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.cc
    #        ${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.h
    #        ${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.cc
    #        ${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.h
    #        ${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.cc
    #        ${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.h
    #        ${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.cc
    #        ${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.h
    #        ${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/router/v2/router.pb.cc
    #        ${bin_include}/envoy/config/filter/http/router/v2/router.pb.h
    #        ${bin_include}/envoy/config/filter/http/router/v2/router.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/router/v2/router.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.cc
    #        ${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.h
    #        ${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.cc
    #        ${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.h
    #        ${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.validate.h
    #        ${bin_include}/envoy/config/filter/http/header_to_metadata/v2/header_to_metadata.pb.cc
    #        ${bin_include}/envoy/config/filter/http/header_to_metadata/v2/header_to_metadata.pb.h
    #        ${bin_include}/envoy/config/filter/network/rbac/v2/rbac.pb.h
    #        ${bin_include}/envoy/config/filter/network/rbac/v2/rbac.pb.cc
    #        ${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.cc
    #        ${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.h
    #        ${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.validate.h
    #        ${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.cc
    #        ${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.h
    #        ${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.validate.h
    #        ${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.cc
    #        ${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.h
    #        ${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.validate.h
    #        ${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.cc
    #        ${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.h
    #        ${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.validate.h
    #        ${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.cc
    #        ${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.h
    #        ${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.validate.h
    #        ${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.cc
    #        ${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.h
    #        ${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.validate.h
    #        ${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.cc
    #        ${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.h
    #        ${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/route.pb.h
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/route.pb.cc
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/route.pb.validate.h
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/route.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/router/router.pb.h
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/router/router.pb.cc
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/router/router.pb.validate.h
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/router/router.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/thrift_proxy.pb.h
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/thrift_proxy.pb.cc
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/thrift_proxy.pb.validate.h
    #        ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/thrift_proxy.pb.validate.cc
    #        ${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.validate.h
    #        ${bin_include}/envoy/config/grpc_credential/v2alpha/file_based_metadata.pb.cc
    #        ${bin_include}/envoy/config/grpc_credential/v2alpha/file_based_metadata.pb.h
    #        ${bin_include}/envoy/config/grpc_credential/v2alpha/file_based_metadata.pb.validate.cc
    #        ${bin_include}/envoy/config/grpc_credential/v2alpha/file_based_metadata.pb.validate.h
    #        ${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.cc
    #        ${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.h
    #        ${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.validate.cc
    #        ${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.validate.h
    #        ${bin_include}/envoy/config/metrics/v2/metrics_service.pb.cc
    #        ${bin_include}/envoy/config/metrics/v2/metrics_service.pb.h
    #        ${bin_include}/envoy/config/metrics/v2/metrics_service.pb.validate.cc
    #        ${bin_include}/envoy/config/metrics/v2/metrics_service.pb.validate.h
    #        ${bin_include}/envoy/config/metrics/v2/stats.pb.cc
    #        ${bin_include}/envoy/config/metrics/v2/stats.pb.h
    #        ${bin_include}/envoy/config/metrics/v2/stats.pb.validate.cc
    #        ${bin_include}/envoy/config/metrics/v2/stats.pb.validate.h
    #        ${bin_include}/envoy/config/overload/v2alpha/overload.pb.h
    #        ${bin_include}/envoy/config/overload/v2alpha/overload.pb.cc
    #        ${bin_include}/envoy/config/overload/v2alpha/overload.pb.validate.h
    #        ${bin_include}/envoy/config/overload/v2alpha/overload.pb.validate.cc
    #        ${bin_include}/envoy/config/ratelimit/v2/rls.pb.cc
    #        ${bin_include}/envoy/config/ratelimit/v2/rls.pb.h
    #        ${bin_include}/envoy/config/ratelimit/v2/rls.pb.validate.cc
    #        ${bin_include}/envoy/config/ratelimit/v2/rls.pb.validate.h
    #        ${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.cc
    #        ${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.h
    #        ${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.validate.cc
    #        ${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.validate.h
    #        ${bin_include}/envoy/config/trace/v2/trace.pb.cc
    #        ${bin_include}/envoy/config/trace/v2/trace.pb.h
    #        ${bin_include}/envoy/config/trace/v2/trace.pb.validate.cc
    #        ${bin_include}/envoy/config/trace/v2/trace.pb.validate.h
    #        ${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.cc
    #        ${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.h
    #        ${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.validate.cc
    #        ${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.validate.h
    #        ${bin_include}/envoy/data/accesslog/v2/accesslog.pb.cc
    #        ${bin_include}/envoy/data/accesslog/v2/accesslog.pb.h
    #        ${bin_include}/envoy/data/accesslog/v2/accesslog.pb.validate.cc
    #        ${bin_include}/envoy/data/accesslog/v2/accesslog.pb.validate.h
    #        ${bin_include}/envoy/data/tap/v2alpha/capture.pb.cc
    #        ${bin_include}/envoy/data/tap/v2alpha/capture.pb.h
    #        ${bin_include}/envoy/data/tap/v2alpha/capture.pb.validate.cc
    #        ${bin_include}/envoy/data/tap/v2alpha/capture.pb.validate.h
    #        ${bin_include}/envoy/data/core/v2alpha/health_check_event.pb.h
    #        ${bin_include}/envoy/data/core/v2alpha/health_check_event.pb.cc
    #        #${bin_include}/envoy/extensions/filters/network/thrift_proxy/v2alpha1/thrift_proxy.pb.cc
    #        #${bin_include}/envoy/extensions/filters/network/thrift_proxy/v2alpha1/thrift_proxy.pb.h
    #        #${bin_include}/envoy/extensions/filters/network/thrift_proxy/v2alpha1/thrift_proxy.pb.validate.cc
    #        #${bin_include}/envoy/extensions/filters/network/thrift_proxy/v2alpha1/thrift_proxy.pb.validate.h
    #        ${bin_include}/envoy/service/accesslog/v2/als.pb.cc
    #        ${bin_include}/envoy/service/accesslog/v2/als.pb.h
    #        ${bin_include}/envoy/service/accesslog/v2/als.pb.validate.cc
    #        ${bin_include}/envoy/service/accesslog/v2/als.pb.validate.h
    #        ${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.cc
    #        ${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.h
    #        ${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.validate.cc
    #        ${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.validate.h
    #        ${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.cc
    #        ${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.h
    #        ${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.validate.cc
    #        ${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.validate.h
    #        ${bin_include}/envoy/service/discovery/v2/ads.pb.cc
    #        ${bin_include}/envoy/service/discovery/v2/ads.pb.h
    #        ${bin_include}/envoy/service/discovery/v2/ads.pb.validate.cc
    #        ${bin_include}/envoy/service/discovery/v2/ads.pb.validate.h
    #        ${bin_include}/envoy/service/discovery/v2/hds.pb.cc
    #        ${bin_include}/envoy/service/discovery/v2/hds.pb.h
    #        ${bin_include}/envoy/service/discovery/v2/hds.pb.validate.cc
    #        ${bin_include}/envoy/service/discovery/v2/hds.pb.validate.h
    #        ${bin_include}/envoy/service/discovery/v2/sds.pb.cc
    #        ${bin_include}/envoy/service/discovery/v2/sds.pb.h
    #        ${bin_include}/envoy/service/discovery/v2/sds.pb.validate.cc
    #        ${bin_include}/envoy/service/discovery/v2/sds.pb.validate.h
    #        ${bin_include}/envoy/service/load_stats/v2/lrs.pb.cc
    #        ${bin_include}/envoy/service/load_stats/v2/lrs.pb.h
    #        ${bin_include}/envoy/service/load_stats/v2/lrs.pb.validate.cc
    #        ${bin_include}/envoy/service/load_stats/v2/lrs.pb.validate.h
    #        ${bin_include}/envoy/service/metrics/v2/metrics_service.pb.cc
    #        ${bin_include}/envoy/service/metrics/v2/metrics_service.pb.h
    #        ${bin_include}/envoy/service/metrics/v2/metrics_service.pb.validate.cc
    #        ${bin_include}/envoy/service/metrics/v2/metrics_service.pb.validate.h
    #        ${bin_include}/envoy/service/ratelimit/v2/rls.pb.cc
    #        ${bin_include}/envoy/service/ratelimit/v2/rls.pb.h
    #        ${bin_include}/envoy/service/ratelimit/v2/rls.pb.validate.cc
    #        ${bin_include}/envoy/service/ratelimit/v2/rls.pb.validate.h
    #        ${bin_include}/envoy/service/trace/v2/trace_service.pb.cc
    #        ${bin_include}/envoy/service/trace/v2/trace_service.pb.h
    #        ${bin_include}/envoy/service/trace/v2/trace_service.pb.validate.cc
    #        ${bin_include}/envoy/service/trace/v2/trace_service.pb.validate.h
    #        ${bin_include}/envoy/type/percent.pb.cc
    #        ${bin_include}/envoy/type/percent.pb.h
    #        ${bin_include}/envoy/type/percent.pb.validate.cc
    #        ${bin_include}/envoy/type/percent.pb.validate.h
    #        ${bin_include}/envoy/type/range.pb.cc
    #        ${bin_include}/envoy/type/range.pb.h
    #        ${bin_include}/envoy/type/range.pb.validate.cc
    #        ${bin_include}/envoy/type/range.pb.validate.h
    #        ${bin_include}/envoy/type/matcher/string.pb.cc
    #        ${bin_include}/envoy/type/matcher/string.pb.h
    #        ${bin_include}/envoy/type/matcher/number.pb.cc
    #        ${bin_include}/envoy/type/matcher/number.pb.h
    #        ${bin_include}/envoy/type/matcher/metadata.pb.cc
    #        ${bin_include}/envoy/type/matcher/metadata.pb.h
    #        ${bin_include}/envoy/type/matcher/value.pb.cc
    #        ${bin_include}/envoy/type/matcher/value.pb.h
    #        ${bin_include}/envoy/type/http_status.pb.cc
    #        ${bin_include}/envoy/type/http_status.pb.h
    #        #${bin_include}/envoy/type/string_match.pb.cc
    #        #${bin_include}/envoy/type/string_match.pb.h
    #        #${bin_include}/envoy/type/string_match.pb.validate.cc
    #        #${bin_include}/envoy/type/string_match.pb.validate.h
    #        ${bin_include}/gogoproto/gogo.pb.cc
    #        ${bin_include}/gogoproto/gogo.pb.h
    #        ${bin_include}/google.golang.org/grpc/health/grpc_health_v1/health.pb.cc
    #        ${bin_include}/google.golang.org/grpc/health/grpc_health_v1/health.pb.h
    #        ${bin_include}/google.golang.org/grpc/health/grpc_health_v1/health.pb.validate.cc
    #        ${bin_include}/google.golang.org/grpc/health/grpc_health_v1/health.pb.validate.h
    #        ${bin_include}/google/api/annotations.pb.cc
    #        ${bin_include}/google/api/annotations.pb.h
    #        ${bin_include}/google/api/http.pb.cc
    #        ${bin_include}/google/api/http.pb.h
    #        ${bin_include}/google/api/httpbody.pb.cc
    #        ${bin_include}/google/api/httpbody.pb.h
    #        ${bin_include}/google/rpc/status.pb.cc
    #        ${bin_include}/google/rpc/status.pb.h
    #        ${bin_include}/lightstep-tracer-common/collector.pb.cc
    #        ${bin_include}/lightstep-tracer-common/collector.pb.h
    #        ${bin_include}/lightstep-tracer-common/lightstep_carrier.pb.cc
    #        ${bin_include}/lightstep-tracer-common/lightstep_carrier.pb.h
    #        ${bin_include}/metrics.pb.cc
    #        ${bin_include}/metrics.pb.h
    #        ${bin_include}/metrics.pb.validate.cc
    #        ${bin_include}/metrics.pb.validate.h
    #        ${bin_include}/source/common/ratelimit/ratelimit.pb.cc
    #        ${bin_include}/source/common/ratelimit/ratelimit.pb.h
    #        ${bin_include}/source/common/ratelimit/ratelimit.pb.validate.cc
    #        ${bin_include}/source/common/ratelimit/ratelimit.pb.validate.h
    #        ${bin_include}/envoy/config/resource_monitor/fixed_heap/v2alpha/fixed_heap.pb.cc
    #        ${bin_include}/envoy/config/resource_monitor/fixed_heap/v2alpha/fixed_heap.pb.h
    #        ${bin_include}/envoy/config/resource_monitor/fixed_heap/v2alpha/fixed_heap.pb.validate.cc
    #        ${bin_include}/envoy/config/resource_monitor/fixed_heap/v2alpha/fixed_heap.pb.validate.h
    #		${bin_include}/test/integration/capture_fuzz.pb.cc
    #		${bin_include}/test/integration/capture_fuzz.pb.h
    #		${bin_include}/test/common/http/conn_manager_impl_fuzz.pb.validate.cc
    #		${bin_include}/test/common/http/header_map_impl_fuzz.pb.cc
    #		${bin_include}/test/common/http/header_map_impl_fuzz.pb.h
    #		${bin_include}/test/common/http/header_map_impl_fuzz.pb.validate.h
    #		${bin_include}/test/common/http/conn_manager_impl_fuzz.pb.cc
    #		${bin_include}/test/common/http/header_map_impl_fuzz.pb.validate.cc
    #		${bin_include}/test/common/http/conn_manager_impl_fuzz.pb.validate.h
    #		${bin_include}/test/common/http/conn_manager_impl_fuzz.pb.h
    #		${bin_include}/test/common/http2/codec_impl_fuzz.pb.cc
    #		${bin_include}/test/common/http2/codec_impl_fuzz.pb.h
    #		${bin_include}/test/common/http2/codec_impl_fuzz.pb.validate.h
    #		${bin_include}/test/common/http2/codec_impl_fuzz.pb.validate.cc
    #		${bin_include}/test/common/access_log/access_log_formatter_fuzz.pb.validate.h
    #		${bin_include}/test/common/access_log/access_log_formatter_fuzz.pb.validate.cc
    #		${bin_include}/test/common/access_log/access_log_formatter_fuzz.pb.cc
    #		${bin_include}/test/common/access_log/access_log_formatter_fuzz.pb.h
    #		${bin_include}/test/common/router/header_parser_fuzz.pb.cc
    #		${bin_include}/test/common/router/route_fuzz.pb.validate.cc
    #		${bin_include}/test/common/router/header_parser_fuzz.pb.h
    #		${bin_include}/test/common/router/header_parser_fuzz.pb.validate.h
    #		${bin_include}/test/common/router/route_fuzz.pb.cc
    #		${bin_include}/test/common/router/route_fuzz.pb.validate.h
    #		${bin_include}/test/common/router/header_parser_fuzz.pb.validate.cc
    #		${bin_include}/test/common/router/route_fuzz.pb.h
    #        ${bin_include}/trace.pb.cc
    #        ${bin_include}/trace.pb.h
    #        ${bin_include}/trace.pb.validate.cc
    #        ${bin_include}/trace.pb.validate.h
    #        ${bin_include}/validate/validate.pb.cc
    #        ${bin_include}/validate/validate.pb.h


    # added new list on 6/22/2018
    #envoy/admin/v2alpha/clusters.pb.validate.cc
    #envoy/admin/v2alpha/clusters.pb.validate.h
    #envoy/admin/v2alpha/metrics.pb.validate.cc
    #envoy/admin/v2alpha/metrics.pb.validate.h
    #envoy/data/core/v2alpha/health_check_event.pb.validate.cc
    #envoy/data/core/v2alpha/health_check_event.pb.validate.h
    #envoy/type/http_status.pb.validate.cc
    #envoy/type/http_status.pb.validate.h
    #envoy/type/matcher/metadata.pb.validate.cc
    #envoy/type/matcher/metadata.pb.validate.h
    #envoy/type/matcher/number.pb.validate.cc
    #envoy/type/matcher/number.pb.validate.h
    #envoy/type/matcher/string.pb.validate.cc
    #envoy/type/matcher/string.pb.validate.h
    #envoy/type/matcher/value.pb.validate.cc
    #envoy/type/matcher/value.pb.validate.h
    #github.com/gogo/protobuf/gogoproto/gogo.pb.validate.cc
    #github.com/gogo/protobuf/gogoproto/gogo.pb.validate.h
    #google.golang.org/genproto/googleapis/api/httpbody/httpbody.pb.validate.cc
    #google.golang.org/genproto/googleapis/api/httpbody/httpbody.pb.validate.h
    #envoy/extensions/filters/network/thrift_proxy/v2alpha1/thrift_proxy.pb.cc
    #envoy/extensions/filters/network/thrift_proxy/v2alpha1/thrift_proxy.pb.h
    #envoy/extensions/filters/network/thrift_proxy/v2alpha1/thrift_proxy.pb.validate.cc
    #envoy/extensions/filters/network/thrift_proxy/v2alpha1/thrift_proxy.pb.validate.h
    ${bin_include}/envoy/admin/v2alpha/clusters.pb.cc
    ${bin_include}/envoy/admin/v2alpha/clusters.pb.h
    ${bin_include}/envoy/admin/v2alpha/config_dump.pb.cc
    ${bin_include}/envoy/admin/v2alpha/config_dump.pb.h
    ${bin_include}/envoy/admin/v2alpha/config_dump.pb.validate.cc
    ${bin_include}/envoy/admin/v2alpha/config_dump.pb.validate.h
    ${bin_include}/envoy/admin/v2alpha/metrics.pb.cc
    ${bin_include}/envoy/admin/v2alpha/metrics.pb.h
	${bin_include}/envoy/admin/v2alpha/memory.pb.validate.h
	${bin_include}/envoy/admin/v2alpha/memory.pb.h
	${bin_include}/envoy/admin/v2alpha/memory.pb.cc
	${bin_include}/envoy/admin/v2alpha/memory.pb.validate.cc
    ${bin_include}/envoy/api/v2/auth/cert.pb.cc
    ${bin_include}/envoy/api/v2/auth/cert.pb.h
    ${bin_include}/envoy/api/v2/auth/cert.pb.validate.cc
    ${bin_include}/envoy/api/v2/auth/cert.pb.validate.h
    ${bin_include}/envoy/api/v2/cds.pb.cc
    ${bin_include}/envoy/api/v2/cds.pb.h
    ${bin_include}/envoy/api/v2/cds.pb.validate.cc
    ${bin_include}/envoy/api/v2/cds.pb.validate.h
    ${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.cc
    ${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.h
    ${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.validate.cc
    ${bin_include}/envoy/api/v2/cluster/circuit_breaker.pb.validate.h
    ${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.cc
    ${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.h
    ${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.validate.cc
    ${bin_include}/envoy/api/v2/cluster/outlier_detection.pb.validate.h
    ${bin_include}/envoy/api/v2/core/address.pb.cc
    ${bin_include}/envoy/api/v2/core/address.pb.h
    ${bin_include}/envoy/api/v2/core/address.pb.validate.cc
    ${bin_include}/envoy/api/v2/core/address.pb.validate.h
    ${bin_include}/envoy/api/v2/core/base.pb.cc
    ${bin_include}/envoy/api/v2/core/base.pb.h
    ${bin_include}/envoy/api/v2/core/base.pb.validate.cc
    ${bin_include}/envoy/api/v2/core/base.pb.validate.h
    ${bin_include}/envoy/api/v2/core/config_source.pb.cc
    ${bin_include}/envoy/api/v2/core/config_source.pb.h
    ${bin_include}/envoy/api/v2/core/config_source.pb.validate.cc
    ${bin_include}/envoy/api/v2/core/config_source.pb.validate.h
    ${bin_include}/envoy/api/v2/core/grpc_service.pb.cc
    ${bin_include}/envoy/api/v2/core/grpc_service.pb.h
    ${bin_include}/envoy/api/v2/core/grpc_service.pb.validate.cc
    ${bin_include}/envoy/api/v2/core/grpc_service.pb.validate.h
    ${bin_include}/envoy/api/v2/core/health_check.pb.cc
    ${bin_include}/envoy/api/v2/core/health_check.pb.h
    ${bin_include}/envoy/api/v2/core/health_check.pb.validate.cc
    ${bin_include}/envoy/api/v2/core/health_check.pb.validate.h
    ${bin_include}/envoy/api/v2/core/http_uri.pb.cc
    ${bin_include}/envoy/api/v2/core/http_uri.pb.h
    ${bin_include}/envoy/api/v2/core/http_uri.pb.validate.cc
    ${bin_include}/envoy/api/v2/core/http_uri.pb.validate.h
    ${bin_include}/envoy/api/v2/core/protocol.pb.cc
    ${bin_include}/envoy/api/v2/core/protocol.pb.h
    ${bin_include}/envoy/api/v2/core/protocol.pb.validate.cc
    ${bin_include}/envoy/api/v2/core/protocol.pb.validate.h
    ${bin_include}/envoy/api/v2/discovery.pb.cc
    ${bin_include}/envoy/api/v2/discovery.pb.h
    ${bin_include}/envoy/api/v2/discovery.pb.validate.cc
    ${bin_include}/envoy/api/v2/discovery.pb.validate.h
    ${bin_include}/envoy/api/v2/eds.pb.cc
    ${bin_include}/envoy/api/v2/eds.pb.h
    ${bin_include}/envoy/api/v2/eds.pb.validate.cc
    ${bin_include}/envoy/api/v2/eds.pb.validate.h
    ${bin_include}/envoy/api/v2/endpoint/endpoint.pb.cc
    ${bin_include}/envoy/api/v2/endpoint/endpoint.pb.h
    ${bin_include}/envoy/api/v2/endpoint/endpoint.pb.validate.cc
    ${bin_include}/envoy/api/v2/endpoint/endpoint.pb.validate.h
    ${bin_include}/envoy/api/v2/endpoint/load_report.pb.cc
    ${bin_include}/envoy/api/v2/endpoint/load_report.pb.h
    ${bin_include}/envoy/api/v2/endpoint/load_report.pb.validate.cc
    ${bin_include}/envoy/api/v2/endpoint/load_report.pb.validate.h
    ${bin_include}/envoy/api/v2/lds.pb.cc
    ${bin_include}/envoy/api/v2/lds.pb.h
    ${bin_include}/envoy/api/v2/lds.pb.validate.cc
    ${bin_include}/envoy/api/v2/lds.pb.validate.h
    ${bin_include}/envoy/api/v2/listener/listener.pb.cc
    ${bin_include}/envoy/api/v2/listener/listener.pb.h
    ${bin_include}/envoy/api/v2/listener/listener.pb.validate.cc
    ${bin_include}/envoy/api/v2/listener/listener.pb.validate.h
    ${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.cc
    ${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.h
    ${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.validate.cc
    ${bin_include}/envoy/api/v2/ratelimit/ratelimit.pb.validate.h
    ${bin_include}/envoy/api/v2/rds.pb.cc
    ${bin_include}/envoy/api/v2/rds.pb.h
    ${bin_include}/envoy/api/v2/rds.pb.validate.cc
    ${bin_include}/envoy/api/v2/rds.pb.validate.h
    ${bin_include}/envoy/api/v2/route/route.pb.cc
    ${bin_include}/envoy/api/v2/route/route.pb.h
    ${bin_include}/envoy/api/v2/route/route.pb.validate.cc
    ${bin_include}/envoy/api/v2/route/route.pb.validate.h
    ${bin_include}/envoy/config/accesslog/v2/als.pb.cc
    ${bin_include}/envoy/config/accesslog/v2/als.pb.h
    ${bin_include}/envoy/config/accesslog/v2/als.pb.validate.cc
    ${bin_include}/envoy/config/accesslog/v2/als.pb.validate.h
    ${bin_include}/envoy/config/accesslog/v2/file.pb.cc
    ${bin_include}/envoy/config/accesslog/v2/file.pb.h
    ${bin_include}/envoy/config/accesslog/v2/file.pb.validate.cc
    ${bin_include}/envoy/config/accesslog/v2/file.pb.validate.h
    ${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.cc
    ${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.h
    ${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.validate.cc
    ${bin_include}/envoy/config/bootstrap/v2/bootstrap.pb.validate.h
    ${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.cc
    ${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.h
    ${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.validate.cc
    ${bin_include}/envoy/config/filter/accesslog/v2/accesslog.pb.validate.h
    ${bin_include}/envoy/config/filter/fault/v2/fault.pb.cc
    ${bin_include}/envoy/config/filter/fault/v2/fault.pb.h
    ${bin_include}/envoy/config/filter/fault/v2/fault.pb.validate.cc
    ${bin_include}/envoy/config/filter/fault/v2/fault.pb.validate.h
    ${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.cc
    ${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.h
    ${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/buffer/v2/buffer.pb.validate.h
    ${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.cc
    ${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.h
    ${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/ext_authz/v2alpha/ext_authz.pb.validate.h
    ${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.cc
    ${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.h
    ${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/fault/v2/fault.pb.validate.h
    ${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.cc
    ${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.h
    ${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/gzip/v2/gzip.pb.validate.h
    ${bin_include}/envoy/config/filter/http/header_to_metadata/v2/header_to_metadata.pb.cc
    ${bin_include}/envoy/config/filter/http/header_to_metadata/v2/header_to_metadata.pb.h
    ${bin_include}/envoy/config/filter/http/header_to_metadata/v2/header_to_metadata.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/header_to_metadata/v2/header_to_metadata.pb.validate.h
    ${bin_include}/envoy/config/filter/network/rbac/v2/rbac.pb.validate.cc
    ${bin_include}/envoy/config/filter/network/rbac/v2/rbac.pb.validate.h
    ${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.cc
    ${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.h
    ${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/health_check/v2/health_check.pb.validate.h
    ${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.cc
    ${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.h
    ${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/ip_tagging/v2/ip_tagging.pb.validate.h
    ${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.cc
    ${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.h
    ${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/jwt_authn/v2alpha/config.pb.validate.h
    ${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.cc
    ${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.h
    ${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/lua/v2/lua.pb.validate.h
    ${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.cc
    ${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.h
    ${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/rate_limit/v2/rate_limit.pb.validate.h
    ${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.cc
    ${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.h
    ${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/rbac/v2/rbac.pb.validate.h
    ${bin_include}/envoy/config/filter/http/router/v2/router.pb.cc
    ${bin_include}/envoy/config/filter/http/router/v2/router.pb.h
    ${bin_include}/envoy/config/filter/http/router/v2/router.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/router/v2/router.pb.validate.h
    ${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.cc
    ${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.h
    ${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/squash/v2/squash.pb.validate.h
    ${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.cc
    ${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.h
    ${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.validate.cc
    ${bin_include}/envoy/config/filter/http/transcoder/v2/transcoder.pb.validate.h
    ${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.cc
    ${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.h
    ${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.validate.cc
    ${bin_include}/envoy/config/filter/network/client_ssl_auth/v2/client_ssl_auth.pb.validate.h
    ${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.cc
    ${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.h
    ${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.validate.cc
    ${bin_include}/envoy/config/filter/network/ext_authz/v2/ext_authz.pb.validate.h
    ${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.cc
    ${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.h
    ${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.validate.cc
    ${bin_include}/envoy/config/filter/network/http_connection_manager/v2/http_connection_manager.pb.validate.h
    ${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.cc
    ${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.h
    ${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.validate.cc
    ${bin_include}/envoy/config/filter/network/mongo_proxy/v2/mongo_proxy.pb.validate.h
    ${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.cc
    ${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.h
    ${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.validate.cc
    ${bin_include}/envoy/config/filter/network/rate_limit/v2/rate_limit.pb.validate.h
    ${bin_include}/envoy/config/filter/network/rbac/v2/rbac.pb.cc
    ${bin_include}/envoy/config/filter/network/rbac/v2/rbac.pb.h
    ${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.cc
    ${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.h
    ${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.validate.cc
    ${bin_include}/envoy/config/filter/network/redis_proxy/v2/redis_proxy.pb.validate.h
    ${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.cc
    ${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.h
    ${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.validate.cc
    ${bin_include}/envoy/config/filter/network/tcp_proxy/v2/tcp_proxy.pb.validate.h
    ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/route.pb.cc
    ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/route.pb.h
    ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/route.pb.validate.cc
    ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/route.pb.validate.h
    ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/thrift_proxy.pb.cc
    ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/thrift_proxy.pb.h
    ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/thrift_proxy.pb.validate.cc
    ${bin_include}/envoy/config/filter/network/thrift_proxy/v2alpha1/thrift_proxy.pb.validate.h
    ${bin_include}/envoy/config/filter/thrift/router/v2alpha1/router.pb.cc
    ${bin_include}/envoy/config/filter/thrift/router/v2alpha1/router.pb.h
    ${bin_include}/envoy/config/filter/thrift/router/v2alpha1/router.pb.validate.cc
    ${bin_include}/envoy/config/filter/thrift/router/v2alpha1/router.pb.validate.h
    ${bin_include}/envoy/config/grpc_credential/v2alpha/file_based_metadata.pb.cc
    ${bin_include}/envoy/config/grpc_credential/v2alpha/file_based_metadata.pb.h
    ${bin_include}/envoy/config/grpc_credential/v2alpha/file_based_metadata.pb.validate.cc
    ${bin_include}/envoy/config/grpc_credential/v2alpha/file_based_metadata.pb.validate.h
    ${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.cc
    ${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.h
    ${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.validate.cc
    ${bin_include}/envoy/config/health_checker/redis/v2/redis.pb.validate.h
    ${bin_include}/envoy/config/metrics/v2/metrics_service.pb.cc
    ${bin_include}/envoy/config/metrics/v2/metrics_service.pb.h
    ${bin_include}/envoy/config/metrics/v2/metrics_service.pb.validate.cc
    ${bin_include}/envoy/config/metrics/v2/metrics_service.pb.validate.h
    ${bin_include}/envoy/config/metrics/v2/stats.pb.cc
    ${bin_include}/envoy/config/metrics/v2/stats.pb.h
    ${bin_include}/envoy/config/metrics/v2/stats.pb.validate.cc
    ${bin_include}/envoy/config/metrics/v2/stats.pb.validate.h
    ${bin_include}/envoy/config/overload/v2alpha/overload.pb.cc
    ${bin_include}/envoy/config/overload/v2alpha/overload.pb.h
    ${bin_include}/envoy/config/overload/v2alpha/overload.pb.validate.cc
    ${bin_include}/envoy/config/overload/v2alpha/overload.pb.validate.h
    ${bin_include}/envoy/config/ratelimit/v2/rls.pb.cc
    ${bin_include}/envoy/config/ratelimit/v2/rls.pb.h
    ${bin_include}/envoy/config/ratelimit/v2/rls.pb.validate.cc
    ${bin_include}/envoy/config/ratelimit/v2/rls.pb.validate.h
    ${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.cc
    ${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.h
    ${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.validate.cc
    ${bin_include}/envoy/config/rbac/v2alpha/rbac.pb.validate.h
    ${bin_include}/envoy/config/resource_monitor/fixed_heap/v2alpha/fixed_heap.pb.cc
    ${bin_include}/envoy/config/resource_monitor/fixed_heap/v2alpha/fixed_heap.pb.h
    ${bin_include}/envoy/config/resource_monitor/fixed_heap/v2alpha/fixed_heap.pb.validate.cc
    ${bin_include}/envoy/config/resource_monitor/fixed_heap/v2alpha/fixed_heap.pb.validate.h
    ${bin_include}/envoy/config/retry/other_priority/other_priority_config.pb.h
    ${bin_include}/envoy/config/retry/other_priority/other_priority_config.pb.cc
    ${bin_include}/envoy/config/retry/other_priority/other_priority_config.pb.validate.h
    ${bin_include}/envoy/config/retry/other_priority/other_priority_config.pb.validate.cc

    ${bin_include}/envoy/config/trace/v2/trace.pb.cc
    ${bin_include}/envoy/config/trace/v2/trace.pb.h
    ${bin_include}/envoy/config/trace/v2/trace.pb.validate.cc
    ${bin_include}/envoy/config/trace/v2/trace.pb.validate.h
    ${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.cc
    ${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.h
    ${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.validate.cc
    ${bin_include}/envoy/config/transport_socket/capture/v2alpha/capture.pb.validate.h
	${bin_include}/envoy/config/resource_monitor/injected_resource/v2alpha/injected_resource.pb.h
	${bin_include}/envoy/config/resource_monitor/injected_resource/v2alpha/injected_resource.pb.cc
	${bin_include}/envoy/config/resource_monitor/injected_resource/v2alpha/injected_resource.pb.validate.cc
	${bin_include}/envoy/config/resource_monitor/injected_resource/v2alpha/injected_resource.pb.validate.h
    ${bin_include}/envoy/data/accesslog/v2/accesslog.pb.cc
    ${bin_include}/envoy/data/accesslog/v2/accesslog.pb.h
    ${bin_include}/envoy/data/accesslog/v2/accesslog.pb.validate.cc
    ${bin_include}/envoy/data/accesslog/v2/accesslog.pb.validate.h
    ${bin_include}/envoy/data/core/v2alpha/health_check_event.pb.cc
    ${bin_include}/envoy/data/core/v2alpha/health_check_event.pb.h
    ${bin_include}/envoy/data/tap/v2alpha/capture.pb.cc
    ${bin_include}/envoy/data/tap/v2alpha/capture.pb.h
    ${bin_include}/envoy/data/tap/v2alpha/capture.pb.validate.cc
    ${bin_include}/envoy/data/tap/v2alpha/capture.pb.validate.h
    ${bin_include}/envoy/service/accesslog/v2/als.pb.cc
    ${bin_include}/envoy/service/accesslog/v2/als.pb.h
    ${bin_include}/envoy/service/accesslog/v2/als.pb.validate.cc
    ${bin_include}/envoy/service/accesslog/v2/als.pb.validate.h
    ${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.cc
    ${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.h
    ${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.validate.cc
    ${bin_include}/envoy/service/auth/v2alpha/attribute_context.pb.validate.h
    ${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.cc
    ${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.h
    ${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.validate.cc
    ${bin_include}/envoy/service/auth/v2alpha/external_auth.pb.validate.h
    ${bin_include}/envoy/service/discovery/v2/ads.pb.cc
    ${bin_include}/envoy/service/discovery/v2/ads.pb.h
    ${bin_include}/envoy/service/discovery/v2/ads.pb.validate.cc
    ${bin_include}/envoy/service/discovery/v2/ads.pb.validate.h
    ${bin_include}/envoy/service/discovery/v2/hds.pb.cc
    ${bin_include}/envoy/service/discovery/v2/hds.pb.h
    ${bin_include}/envoy/service/discovery/v2/hds.pb.validate.cc
    ${bin_include}/envoy/service/discovery/v2/hds.pb.validate.h
    ${bin_include}/envoy/service/discovery/v2/sds.pb.cc
    ${bin_include}/envoy/service/discovery/v2/sds.pb.h
    ${bin_include}/envoy/service/discovery/v2/sds.pb.validate.cc
    ${bin_include}/envoy/service/discovery/v2/sds.pb.validate.h
    ${bin_include}/envoy/service/load_stats/v2/lrs.pb.cc
    ${bin_include}/envoy/service/load_stats/v2/lrs.pb.h
    ${bin_include}/envoy/service/load_stats/v2/lrs.pb.validate.cc
    ${bin_include}/envoy/service/load_stats/v2/lrs.pb.validate.h
    ${bin_include}/envoy/service/metrics/v2/metrics_service.pb.cc
    ${bin_include}/envoy/service/metrics/v2/metrics_service.pb.h
    ${bin_include}/envoy/service/metrics/v2/metrics_service.pb.validate.cc
    ${bin_include}/envoy/service/metrics/v2/metrics_service.pb.validate.h
    ${bin_include}/envoy/service/ratelimit/v2/rls.pb.cc
    ${bin_include}/envoy/service/ratelimit/v2/rls.pb.h
    ${bin_include}/envoy/service/ratelimit/v2/rls.pb.validate.cc
    ${bin_include}/envoy/service/ratelimit/v2/rls.pb.validate.h
    ${bin_include}/envoy/service/trace/v2/trace_service.pb.cc
    ${bin_include}/envoy/service/trace/v2/trace_service.pb.h
    ${bin_include}/envoy/service/trace/v2/trace_service.pb.validate.cc
    ${bin_include}/envoy/service/trace/v2/trace_service.pb.validate.h
    ${bin_include}/envoy/type/http_status.pb.cc
    ${bin_include}/envoy/type/http_status.pb.h
    ${bin_include}/envoy/type/matcher/metadata.pb.cc
    ${bin_include}/envoy/type/matcher/metadata.pb.h
    ${bin_include}/envoy/type/matcher/number.pb.cc
    ${bin_include}/envoy/type/matcher/number.pb.h
    ${bin_include}/envoy/type/matcher/string.pb.cc
    ${bin_include}/envoy/type/matcher/string.pb.h
    ${bin_include}/envoy/type/matcher/value.pb.cc
    ${bin_include}/envoy/type/matcher/value.pb.h
    ${bin_include}/envoy/type/percent.pb.cc
    ${bin_include}/envoy/type/percent.pb.h
    ${bin_include}/envoy/type/percent.pb.validate.cc
    ${bin_include}/envoy/type/percent.pb.validate.h
    ${bin_include}/envoy/type/range.pb.cc
    ${bin_include}/envoy/type/range.pb.h
    ${bin_include}/envoy/type/range.pb.validate.cc
    ${bin_include}/envoy/type/range.pb.validate.h
    ${bin_include}/gogoproto/gogo.pb.cc
    ${bin_include}/gogoproto/gogo.pb.h
    ${bin_include}/google.golang.org/grpc/health/grpc_health_v1/health.pb.cc
    ${bin_include}/google.golang.org/grpc/health/grpc_health_v1/health.pb.h
    ${bin_include}/google.golang.org/grpc/health/grpc_health_v1/health.pb.validate.cc
    ${bin_include}/google.golang.org/grpc/health/grpc_health_v1/health.pb.validate.h
    ${bin_include}/google/api/annotations.pb.cc
    ${bin_include}/google/api/annotations.pb.h
    ${bin_include}/google/api/http.pb.cc
    ${bin_include}/google/api/http.pb.h
    ${bin_include}/google/api/httpbody.pb.cc
    ${bin_include}/google/api/httpbody.pb.h
    ${bin_include}/google/rpc/status.pb.cc
    ${bin_include}/google/rpc/status.pb.h
    ${bin_include}/lightstep-tracer-common/collector.pb.cc
    ${bin_include}/lightstep-tracer-common/collector.pb.h
    ${bin_include}/lightstep-tracer-common/lightstep_carrier.pb.cc
    ${bin_include}/lightstep-tracer-common/lightstep_carrier.pb.h
    ${bin_include}/metrics.pb.cc
    ${bin_include}/metrics.pb.h
    ${bin_include}/metrics.pb.validate.cc
    ${bin_include}/metrics.pb.validate.h
    ${bin_include}/source/common/ratelimit/ratelimit.pb.cc
    ${bin_include}/source/common/ratelimit/ratelimit.pb.h
    ${bin_include}/source/common/ratelimit/ratelimit.pb.validate.cc
    ${bin_include}/source/common/ratelimit/ratelimit.pb.validate.h
    #    ${bin_include}/test/proto/bookstore.pb.h
    #    ${bin_include}/test/proto/bookstore.pb.cc
    #    ${bin_include}/test/proto/bookstore.pb.validate.h
    #    ${bin_include}/test/proto/bookstore.pb.validate.cc
    #    ${bin_include}/test/proto/helloworld.pb.h
    #    ${bin_include}/test/proto/helloworld.pb.cc
    #    ${bin_include}/test/proto/helloworld.pb.validate.h
    #    ${bin_include}/test/proto/helloworld.pb.validate.cc
    ${bin_include}/test/common/access_log/access_log_formatter_fuzz.pb.cc
    ${bin_include}/test/common/access_log/access_log_formatter_fuzz.pb.h
    ${bin_include}/test/common/access_log/access_log_formatter_fuzz.pb.validate.cc
    ${bin_include}/test/common/access_log/access_log_formatter_fuzz.pb.validate.h
    ${bin_include}/test/common/http/conn_manager_impl_fuzz.pb.cc
    ${bin_include}/test/common/http/conn_manager_impl_fuzz.pb.h
    ${bin_include}/test/common/http/conn_manager_impl_fuzz.pb.validate.cc
    ${bin_include}/test/common/http/conn_manager_impl_fuzz.pb.validate.h
    ${bin_include}/test/common/http/header_map_impl_fuzz.pb.cc
    ${bin_include}/test/common/http/header_map_impl_fuzz.pb.h
    ${bin_include}/test/common/http/header_map_impl_fuzz.pb.validate.cc
    ${bin_include}/test/common/http/header_map_impl_fuzz.pb.validate.h
    ${bin_include}/test/common/http/utility_fuzz.pb.cc
    ${bin_include}/test/common/http/utility_fuzz.pb.h
    ${bin_include}/test/common/http/utility_fuzz.pb.validate.cc
    ${bin_include}/test/common/http/utility_fuzz.pb.validate.h
    ${bin_include}/test/common/http2/codec_impl_fuzz.pb.cc
    ${bin_include}/test/common/http2/codec_impl_fuzz.pb.h
    ${bin_include}/test/common/http2/codec_impl_fuzz.pb.validate.cc
    ${bin_include}/test/common/http2/codec_impl_fuzz.pb.validate.h
    ${bin_include}/test/common/router/header_parser_fuzz.pb.cc
    ${bin_include}/test/common/router/header_parser_fuzz.pb.h
    ${bin_include}/test/common/router/header_parser_fuzz.pb.validate.cc
    ${bin_include}/test/common/router/header_parser_fuzz.pb.validate.h
    ${bin_include}/test/common/router/route_fuzz.pb.cc
    ${bin_include}/test/common/router/route_fuzz.pb.h
    ${bin_include}/test/common/router/route_fuzz.pb.validate.cc
    ${bin_include}/test/common/router/route_fuzz.pb.validate.h
    ${bin_include}/test/fuzz/common.pb.cc
    ${bin_include}/test/fuzz/common.pb.h
    ${bin_include}/test/fuzz/common.pb.validate.cc
    ${bin_include}/test/fuzz/common.pb.validate.h
    ${bin_include}/test/integration/capture_fuzz.pb.cc
    ${bin_include}/test/integration/capture_fuzz.pb.h
    ${bin_include}/trace.pb.cc
    ${bin_include}/trace.pb.h
    ${bin_include}/trace.pb.validate.cc
    ${bin_include}/trace.pb.validate.h
    ${bin_include}/validate/validate.pb.cc
    ${bin_include}/validate/validate.pb.h
)

set_source_files_properties(${proto_cc_h_list} PROPERTIES GENERATED TRUE)

set(CMAKE_C_FLAGS "-D_GNU_SOURCE -ggdb")
add_definitions(${CMAKE_C_FLAGS})

#TODO Add this library as a dependency on outside target that gets built with all
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include/api)
include_directories(${CMAKE_SOURCE_DIR}/ev/external/protoc-gen-validate/)
include_directories(${ev_external_dir}/grpc/third_party/protobuf/src/)
#message(STATUS "blahblahblah ${proto_cc_h_list}")
set (CMAKE_CXX_STANDARD 11)
add_library(${ev_protos_project} STATIC ${proto_cc_h_list})
add_dependencies(${ev_protos_project} target_gen)
set_target_properties(${ev_protos_project} PROPERTIES LINKER_LANGUAGE CXX)




















# bazel configure
# bazel build configure
#set(bazel_build_cmd bazel build -c opt -j 4 //envoy/api/v2:all)
#
#ExternalProject_Add(${project_name}
#    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
#    CONFIGURE_COMMAND echo "No configure command"
#    BUILD_IN_SOURCE     1
#    BUILD_COMMAND ${bazel_build_cmd}
#    INSTALL_COMMAND echo "No install cmd"
#)



