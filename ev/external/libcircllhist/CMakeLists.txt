include(ExternalProject)

file(GLOB_RECURSE
    sources
    ${CMAKE_SOURCE_DIR}/ev/external/libcircllhist/*.h
    ${CMAKE_SOURCE_DIR}/ev/external/libcircllhist/*.c
)

set(configure_command   ./configure)
set(source_dir          ${CMAKE_SOURCE_DIR}/ev/external/libcircllhist)
set(install_cmd         mv
                        ${CMAKE_SOURCE_DIR}/ev/external/libcircllhist/src/libcircllhist.a
                        ${CMAKE_BINARY_DIR}/ev_lib/)

ExternalProject_Add(libcircllhist
    SOURCE_DIR ${source_dir}
    CONFIGURE_COMMAND cd ${source_dir} && ${configure_command} && mkdir -p ${CMAKE_BINARY_DIR}/ev_lib/
    BUILD_COMMAND make -C ${source_dir}
    INSTALL_COMMAND echo "moving file liblibcircllhist.a" && ${install_cmd}
    STEP_TARGETS configure_libcircllhist
)

ExternalProject_Add_Step(libcircllhist configure_libcircllhist
    COMMAND cd ${source_dir} && ${configure_command}
    COMMENT "configure libcircllhist"
    DEPENDS ${sources}
    DEPENDERS configure # configure step of project libcircllhist depends on this step
    DEPENDEES update    # this step depends on update step
)

set_target_properties(libcircllhist
                        PROPERTIES
                        IMPORTED_LOCATION
                        ${CMAKE_BINARY_DIR}/ev/external/libcircllhist.a)
