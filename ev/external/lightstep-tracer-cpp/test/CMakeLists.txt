macro(_lightstep_test TEST_NAME)
  add_executable(${TEST_NAME} ${ARGN})
  target_link_libraries(${TEST_NAME} lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})
  target_compile_options(${TEST_NAME} PUBLIC "-g")
  add_test(${TEST_NAME} ${TEST_NAME})

  if (LIGHTSTEP_USE_ASAN)
    add_executable(${TEST_NAME}_asan ${ARGN})
    target_link_libraries(${TEST_NAME}_asan lightstep_tracer_asan 
                          ${LIGHTSTEP_LINK_LIBRARIES})
    target_compile_options(${TEST_NAME}_asan PUBLIC ${ASAN_CXX_FLAGS})
    set_target_properties(${TEST_NAME}_asan 
      PROPERTIES LINK_FLAGS "${ASAN_LD_FLAGS}")
    add_test(${TEST_NAME}_asan ${TEST_NAME}_asan)
  endif()

  if (LIGHTSTEP_USE_TSAN)
    add_executable(${TEST_NAME}_tsan ${ARGN})
    target_link_libraries(${TEST_NAME}_tsan lightstep_tracer_tsan 
                          ${LIGHTSTEP_LINK_LIBRARIES})
    target_compile_options(${TEST_NAME}_tsan PUBLIC ${TSAN_CXX_FLAGS})
    set_target_properties(${TEST_NAME}_tsan 
      PROPERTIES LINK_FLAGS "${TSAN_LD_FLAGS}")
    add_test(${TEST_NAME}_tsan ${TEST_NAME}_tsan)
  endif()
endmacro()

_lightstep_test(tracer_test tracer_test.cpp 
                            utility.cpp)
_lightstep_test(utility_test utility_test.cpp)
_lightstep_test(logger_test logger_test.cpp)
_lightstep_test(propagation_test propagation_test.cpp)
_lightstep_test(auto_recorder_test auto_recorder_test.cpp
                                   utility.cpp
                                   in_memory_sync_transporter.cpp
                                   testing_condition_variable_wrapper.cpp)
_lightstep_test(manual_recorder_test manual_recorder_test.cpp
                                     utility.cpp
                                     in_memory_async_transporter.cpp
                                     testing_condition_variable_wrapper.cpp)
