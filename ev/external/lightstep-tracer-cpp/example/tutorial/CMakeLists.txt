if (LIGHTSTEP_USE_GRPC)
  add_executable(tutorial tutorial.cpp)
  target_link_libraries(tutorial lightstep_tracer ${LIGHTSTEP_LINK_LIBRARIES})
endif()
