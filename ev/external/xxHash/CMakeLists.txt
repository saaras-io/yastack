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
    CONFIGURE_COMMAND echo "No configure command"
    BUILD_COMMAND make
    #INSTALL_COMMAND sudo make install
    INSTALL_COMMAND echo "No install cmd"
)
