find_program(CLANG_TIDY_EXE NAMES "clang-tidy" 
                            DOC "Path to clang-tidy executable")
if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" 
"-checks=*,\
-clang-analyzer-alpha.*,\
-llvm-include-order,\
-google-runtime-references,\
-google-build-using-namespace,\
-cppcoreguidelines-pro-type-vararg")
endif()

macro(_apply_clang_tidy_if_available TARGET)
  if (CLANG_TIDY_EXE)
    set_target_properties(${TARGET} PROPERTIES
                                           CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
  endif()
endmacro()
