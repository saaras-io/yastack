set(PROTO_PATH "${CMAKE_SOURCE_DIR}/lightstep-tracer-common")

set(GOOGLE_API_HTTP_PROTO ${PROTO_PATH}/third_party/googleapis/google/api/http.proto)
set(GOOGLE_API_ANNOTATIONS_PROTO ${PROTO_PATH}/third_party/googleapis/google/api/annotations.proto)
set(COLLECTOR_PROTO ${PROTO_PATH}/collector.proto)
set(LIGHTSTEP_CARRIER_PROTO ${PROTO_PATH}/lightstep_carrier.proto)
set(GENERATED_PROTOBUF_PATH ${CMAKE_BINARY_DIR}/generated/lightstep-tracer-common)
file(MAKE_DIRECTORY ${GENERATED_PROTOBUF_PATH})

set(GOOGLE_API_HTTP_PB_CPP_FILE ${GENERATED_PROTOBUF_PATH}/google/api/http.pb.cc)
set(GOOGLE_API_HTTP_PB_H_FILE ${GENERATED_PROTOBUF_PATH}/google/api/http.pb.h)

set(GOOGLE_API_ANNOTATIONS_PB_CPP_FILE ${GENERATED_PROTOBUF_PATH}/google/api/annotations.pb.cc)
set(GOOGLE_API_ANNOTATIONS_PB_H_FILE ${GENERATED_PROTOBUF_PATH}/google/api/annotations.pb.h)

set(COLLECTOR_PB_CPP_FILE ${GENERATED_PROTOBUF_PATH}/collector.pb.cc)
set(COLLECTOR_PB_H_FILE ${GENERATED_PROTOBUF_PATH}/collector.pb.h)
set(COLLECTOR_GRPC_PB_CPP_FILE ${GENERATED_PROTOBUF_PATH}/collector.grpc.pb.cc)
set(COLLECTOR_GRPC_PB_H_FILE ${GENERATED_PROTOBUF_PATH}/collector.grpc.pb.h)

set(LIGHTSTEP_CARRIER_PB_CPP_FILE ${GENERATED_PROTOBUF_PATH}/lightstep_carrier.pb.cc)
set(LIGHTSTEP_CARRIER_PB_H_FILE ${GENERATED_PROTOBUF_PATH}/lightstep_carrier.pb.h)

set(PROTOBUF_INCLUDE_FLAGS "-I${PROTO_PATH}/third_party/googleapis")
foreach(IMPORT_DIR ${PROTOBUF_IMPORT_DIRS})
  list(APPEND PROTOBUF_INCLUDE_FLAGS "-I${IMPORT_DIR}")
endforeach()

add_custom_command(
  OUTPUT ${GOOGLE_API_HTTP_PB_H_FILE}
         ${GOOGLE_API_HTTP_PB_CPP_FILE}
         ${GOOGLE_API_ANNOTATIONS_PB_H_FILE}
         ${GOOGLE_API_ANNOTATIONS_PB_CPP_FILE}
         ${COLLECTOR_PB_H_FILE}
         ${COLLECTOR_PB_CPP_FILE}
         ${LIGHTSTEP_CARRIER_PB_H_FILE}
         ${LIGHTSTEP_CARRIER_PB_CPP_FILE}
 COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
 ARGS "--proto_path=${PROTO_PATH}/third_party/googleapis"
      ${PROTOBUF_INCLUDE_FLAGS}
      "--cpp_out=${GENERATED_PROTOBUF_PATH}"
      ${GOOGLE_API_HTTP_PROTO}
      ${GOOGLE_API_ANNOTATIONS_PROTO}
 COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
 ARGS "--proto_path=${PROTO_PATH}"
      ${PROTOBUF_INCLUDE_FLAGS}
      "--cpp_out=${GENERATED_PROTOBUF_PATH}"
      ${COLLECTOR_PROTO}
      ${LIGHTSTEP_CARRIER_PROTO}
)

include_directories(SYSTEM ${GENERATED_PROTOBUF_PATH})
include_directories(SYSTEM ${GENERATED_PROTOBUF_PATH}/../)

if (LIGHTSTEP_USE_GRPC)
  add_custom_command(
      OUTPUT ${COLLECTOR_GRPC_PB_H_FILE}
             ${COLLECTOR_GRPC_PB_CPP_FILE}
      COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS "--proto_path=${PROTO_PATH}"
           "--proto_path=${PROTO_PATH}/third_party/googleapis"
           ${PROTOBUF_INCLUDE_FLAGS}
           "--grpc_out=${GENERATED_PROTOBUF_PATH}"
           "--plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}"
           "${COLLECTOR_PROTO}"
      )
  
  add_library(lightstep_protobuf OBJECT ${GOOGLE_API_HTTP_PB_CPP_FILE}
                                        ${GOOGLE_API_ANNOTATIONS_PB_CPP_FILE}
                                        ${COLLECTOR_PB_CPP_FILE}
                                        ${COLLECTOR_GRPC_PB_CPP_FILE}
                                        ${LIGHTSTEP_CARRIER_PB_CPP_FILE})
else()
  add_library(lightstep_protobuf OBJECT ${GOOGLE_API_HTTP_PB_CPP_FILE}
                                        ${GOOGLE_API_ANNOTATIONS_PB_CPP_FILE}
                                        ${COLLECTOR_PB_CPP_FILE}
                                        ${LIGHTSTEP_CARRIER_PB_CPP_FILE})
endif()

if (BUILD_SHARED_LIBS)
  set_property(TARGET lightstep_protobuf PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
