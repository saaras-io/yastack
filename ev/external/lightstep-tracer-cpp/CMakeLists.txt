cmake_minimum_required(VERSION 3.1)

project(lightstep-tracer)

# ==============================================================================
# Version information

set(LIGHTSTEP_VERSION_MAJOR "0")
set(LIGHTSTEP_VERSION_MINOR "6")
set(LIGHTSTEP_VERSION_PATCH "1")
set(LIGHTSTEP_VERSION_STRING
  "${LIGHTSTEP_VERSION_MAJOR}.${LIGHTSTEP_VERSION_MINOR}.${LIGHTSTEP_VERSION_PATCH}")

# ==============================================================================
# Set up cpack

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY 
                  "A LightStep implementation of the C++ OpenTracing API")
set(CPACK_PACKAGE_VENDOR "lightstep.com")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_PACKAGE_VERSION_MAJOR ${LIGHTSTEP_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LIGHTSTEP_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LIGHTSTEP_VERSION_PATCH})
include(CPack)

# ==============================================================================
# Set up options

option(WITH_ASAN "Generate tests using address-sanitizer." OFF)
option(WITH_TSAN "Generate tests using thread-sanitizer." OFF)
#option(WITH_GRPC "Build with support for gRPC." OFF)
option(ENABLE_LINTING "Run clang-tidy on sources if available." OFF)
option(HEADERS_ONLY "Only generate config.h and version.h." OFF)

#if (WITH_GRPC)
#  set(LIGHTSTEP_USE_GRPC 1)
#endif()

# ==============================================================================
# Set up generated header files config.h and version.h

configure_file(version.h.in include/lightstep/version.h)
configure_file(config.h.in include/lightstep/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/lightstep 
        DESTINATION include)

if(HEADERS_ONLY)
  return()
endif()

# ==============================================================================
# Configure compiler warnings

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(WARNING_CXX_FLAGS -Weverything 
    -Wno-c++98-compat 
    -Wno-c++98-compat-bind-to-temporary-copy 
    -Wno-weak-vtables 
    -Wno-exit-time-destructors 
    -Wno-global-constructors 
    -Wno-sign-conversion
    -Wno-padded
    -Wno-switch-enum
    -Wno-disabled-macro-expansion)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(WARNING_CXX_FLAGS -Wall -Wextra)
endif()

# ==============================================================================
# Find packages

find_package(Protobuf REQUIRED)

# for all other opentracing includes
set(OPENTRACING_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/ev/external/opentracing-cpp/include/)
# for opentracing/version.h which is generated
set(OPENTRACING_GEN_INCLUDE_DIR ${CMAKE_BINARY_DIR}/ev/external/opentracing-cpp/include/)
# for opentracing 3rd-party includes
set(OPENTRACING_THIRDPARTY_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/ev/external/opentracing-cpp/3rd_party/include/)

if (NOT DEFINED OPENTRACING_INCLUDE_DIR)
  find_path(OPENTRACING_INCLUDE_DIR NAMES opentracing/tracer.h)
endif()

# Ensure that opentracing-cpp is compiled before lightstep, so that library is found
set(OPENTRACING_LIBRARY ${CMAKE_BINARY_DIR}/external/opentracing-cpp/libopentracing.a)

if (NOT DEFINED OPENTRACING_LIBRARY)
  find_library(OPENTRACING_LIBRARY opentracing)
endif()

include_directories(SYSTEM ${OPENTRACING_INCLUDE_DIR})
include_directories(SYSTEM ${OPENTRACING_GEN_INCLUDE_DIR})
include_directories(SYSTEM ${OPENTRACING_THIRDPARTY_INCLUDE_DIR})

include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIRS})

set(LIGHTSTEP_LINK_LIBRARIES ${OPENTRACING_LIBRARY}
                             ${PROTOBUF_LIBRARIES})

#if (WITH_GRPC)                           
#    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
#  if (NOT GRPC_CPP_PLUGIN)
#    message(FATAL_ERROR "grpc_cpp_plugin not found!")
#  endif()
#  find_package(PkgConfig REQUIRED)
#  pkg_search_module(GRPC REQUIRED grpc)
#  pkg_search_module(GRPCPP REQUIRED grpc++)
#  list(APPEND LIGHTSTEP_LINK_LIBRARIES ${GRPC_LDFLAGS} ${GRPCPP_LDFLAGS})  
#  include_directories(SYSTEM ${GRPC_INCLUDE_DIRS} ${GRPCPP_INCLUDE_DIRS})
#endif()

# ==============================================================================
# Configure sanitizers

if (WITH_ASAN AND (("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") OR
  ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND
   "${COMPILER_VERSION}" VERSION_GREATER "4.8")))
   set(LIGHTSTEP_USE_ASAN ON)
   set(ASAN_CXX_FLAGS -O1 -g -fsanitize=address -fno-omit-frame-pointer)
   set(ASAN_LD_FLAGS -fsanitize=address)
endif()

if (WITH_TSAN AND (("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") OR
  ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND
   "${COMPILER_VERSION}" VERSION_GREATER "4.8")))
   set(LIGHTSTEP_USE_TSAN ON)
   set(TSAN_CXX_FLAGS -O1 -g -fsanitize=thread)
   set(TSAN_LD_FLAGS -fsanitize=thread)
endif()

# ==============================================================================
# Build LightStep tracer library

add_subdirectory(3rd_party)
include_directories(SYSTEM ${LIGHTSTEP_THIRD_PARTY_INCLUDES})

include_directories(include)
include_directories(${CMAKE_BINARY_DIR}/include)
install(DIRECTORY include/lightstep DESTINATION include)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

# ev/external/data-plane-api/CMakeLists.txt copies these protos in ${CMAKE_BINARY_DIR}/protos/
# while maintaining the correct directory structure and generates them.
# Generating them here is not required since it duplicates the protos
#include(LightStepProtobuf)

set(LIGHTSTEP_SRCS src/utility.cpp
                   src/in_memory_stream.cpp
                   src/logger.cpp
                   src/propagation.cpp
                   src/binary_carrier.cpp
                   src/grpc_transporter.cpp
                   src/report_builder.cpp
                   src/manual_recorder.cpp
                   src/auto_recorder.cpp
                   src/lightstep_span_context.cpp
                   src/lightstep_span.cpp
                   src/lightstep_tracer_impl.cpp
                   src/transporter.cpp
                   src/tracer.cpp)

#add_library(lightstep_tracer $<TARGET_OBJECTS:lightstep_protobuf>
#                             $<TARGET_OBJECTS:lightstep_3rd_party>
#                             ${LIGHTSTEP_SRCS})

add_library(lightstep_tracer $<TARGET_OBJECTS:lightstep_3rd_party>
                             ${LIGHTSTEP_SRCS})
target_compile_options(lightstep_tracer PUBLIC ${WARNING_CXX_FLAGS})
#if (ENABLE_LINTING)
#  include(LightStepClangTidy)
#  _apply_clang_tidy_if_available(lightstep_tracer)
#endif()

install(TARGETS lightstep_tracer
        LIBRARY DESTINATION lib 
        ARCHIVE DESTINATION lib)

target_link_libraries(lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})

# ==============================================================================
# Build optional sanitized versions of the LightStep tracer

if (LIGHTSTEP_USE_ASAN)
  add_library(lightstep_tracer_asan $<TARGET_OBJECTS:lightstep_protobuf>
                                    ${LIGHTSTEP_SRCS})
  target_compile_options(lightstep_tracer_asan PUBLIC ${ASAN_CXX_FLAGS})
  target_link_libraries(lightstep_tracer_asan ${LIGHTSTEP_LINK_LIBRARIES})
  set_target_properties(lightstep_tracer_asan 
    PROPERTIES LINK_FLAGS "${ASAN_LD_FLAGS}")
endif()

if (LIGHTSTEP_USE_TSAN)
  add_library(lightstep_tracer_tsan $<TARGET_OBJECTS:lightstep_protobuf>
                                    ${LIGHTSTEP_SRCS})
  target_compile_options(lightstep_tracer_tsan PUBLIC ${TSAN_CXX_FLAGS})
  target_link_libraries(lightstep_tracer_tsan ${LIGHTSTEP_LINK_LIBRARIES})
  set_target_properties(lightstep_tracer_tsan 
    PROPERTIES LINK_FLAGS "${TSAN_LD_FLAGS}")
endif()

# ==============================================================================
# Build tests and examples

#include(CTest)
#if (BUILD_TESTING)
#  add_subdirectory(test)
#endif()
add_subdirectory(example)
