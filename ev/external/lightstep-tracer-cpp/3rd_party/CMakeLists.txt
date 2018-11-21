set(LIGHTSTEP_THIRD_PARTY_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/catch/include 
                                   ${CMAKE_CURRENT_SOURCE_DIR}/base64/include
)
set(LIGHTSTEP_THIRD_PARTY_INCLUDES ${LIGHTSTEP_THIRD_PARTY_INCLUDES} 
                                   PARENT_SCOPE)

include_directories(SYSTEM ${LIGHTSTEP_THIRD_PARTY_INCLUDES})
add_library(lightstep_3rd_party OBJECT base64/src/base64.cpp)

if (BUILD_SHARED_LIBS)
  set_property(TARGET lightstep_3rd_party PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
