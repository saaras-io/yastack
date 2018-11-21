include(ExternalProject)

get_filename_component(project_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${project_name})

file(GLOB_RECURSE
    sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
)

ExternalProject_Add(${project_name}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    BUILD_IN_SOURCE     1
    CONFIGURE_COMMAND mkdir -p ${CMAKE_BINARY_DIR}/external/googleapis/
    BUILD_COMMAND make -C ${CMAKE_SOURCE_DIR}/ev/external/googleapis/ OUTPUT=.${CMAKE_BINARY_DIR}/external/googleapis/ GRPCPLUGIN=${CMAKE_BINARY_DIR}/ev/external/grpc/grpc_cpp_plugin
    #INSTALL_COMMAND sudo make install
    INSTALL_COMMAND echo "No install cmd"
    ALL
)
