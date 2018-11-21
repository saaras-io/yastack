set(CMAKE_C_FLAGS "-D_GNU_SOURCE -ggdb")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++14")
add_definitions(${CMAKE_C_FLAGS})

get_filename_component(project_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${project_name})

include(${CMAKE_SOURCE_DIR}/ev/CMakeIncludes.txt)
include_directories(${CMAKE_SOURCE_DIR}/ev/external/cctz/include)

file(GLOB_RECURSE
    sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
)

add_library(${project_name} STATIC ${sources})
