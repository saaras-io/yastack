
##### SECTION ##### Setup project and project-properties #####
project(falcon)

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Limited configurations" FORCE)

#TODO: Build both debug and release builds
# Build type - Debug|Release|RelWithDebInfo|MinSizeRel
set_property(GLOBAL		PROPERTY	CMAKE_BUILD_TYPE	        Debug)

# Print the target tree in the beginning
set_property(GLOBAL     PROPERTY    GLOBAL_DEPENDS_DEBUG_MODE   1)

SET(CMAKE_VERBOSE_MAKEFILE ON)

##### END-SECTION ##### Setup project and project-properties #####

##### SECTION ##### Determine host environment  #####

cmake_host_system_information(RESULT lc                 QUERY NUMBER_OF_LOGICAL_CORES)
cmake_host_system_information(RESULT pc                 QUERY NUMBER_OF_PHYSICAL_CORES)
cmake_host_system_information(RESULT hname              QUERY HOSTNAME)
cmake_host_system_information(RESULT fqdn               QUERY FQDN)
cmake_host_system_information(RESULT total_vm           QUERY TOTAL_VIRTUAL_MEMORY)
cmake_host_system_information(RESULT avai_vm            QUERY AVAILABLE_VIRTUAL_MEMORY)
cmake_host_system_information(RESULT total_pm           QUERY TOTAL_PHYSICAL_MEMORY)
cmake_host_system_information(RESULT avai_pm            QUERY AVAILABLE_PHYSICAL_MEMORY)
cmake_host_system_information(RESULT is_64b             QUERY IS_64BIT)
cmake_host_system_information(RESULT has_fpu            QUERY HAS_FPU)
cmake_host_system_information(RESULT has_mmx            QUERY HAS_MMX)
cmake_host_system_information(RESULT has_mmx_plus       QUERY HAS_MMX_PLUS)
cmake_host_system_information(RESULT has_sse            QUERY HAS_SSE)
cmake_host_system_information(RESULT has_sse2           QUERY HAS_SSE2)
cmake_host_system_information(RESULT has_sse_fp         QUERY HAS_SSE_FP)
cmake_host_system_information(RESULT has_sse_mmx        QUERY HAS_SSE_MMX)
cmake_host_system_information(RESULT has_amd_3dnow      QUERY HAS_AMD_3DNOW)
cmake_host_system_information(RESULT has_amd_3dnow_plus QUERY HAS_AMD_3DNOW_PLUS)
cmake_host_system_information(RESULT has_ia64           QUERY HAS_IA64)
cmake_host_system_information(RESULT has_snum           QUERY HAS_SERIAL_NUMBER)
cmake_host_system_information(RESULT has_proc_snum      QUERY PROCESSOR_SERIAL_NUMBER)
cmake_host_system_information(RESULT proc_name          QUERY PROCESSOR_NAME)
cmake_host_system_information(RESULT proc_desc          QUERY PROCESSOR_DESCRIPTION)
cmake_host_system_information(RESULT os_name            QUERY OS_NAME)
cmake_host_system_information(RESULT os_rel             QUERY OS_RELEASE)
cmake_host_system_information(RESULT os_ver             QUERY OS_VERSION)
cmake_host_system_information(RESULT os_platform        QUERY OS_PLATFORM)

message("logical cores      = ${lc}")    
message("physical cores     = ${pc}")    
message("hname              = ${hname}")    
message("fqdn               = ${fqdn}")    
message("total_vm           = ${total_vm}")    
message("avai_vm            = ${avai_vm}")    
message("total_pm           = ${total_pm}")    
message("avai_pm            = ${avai_pm}")    
message("is_64b             = ${is_64b}")    
message("has_fpu            = ${has_fpu}")    
message("has_mmx            = ${has_mmx}")    
message("has_mmx_plus       = ${has_mmx_plus}")    
message("has_sse            = ${has_sse}")    
message("has_sse2           = ${has_sse2}")    
message("has_sse_fp         = ${has_sse_fp}")    
message("has_sse_mmx        = ${has_sse_mmx}")    
message("has_amd_3dnow      = ${has_amd_3dnow}")    
message("has_amd_3dnow_plus = ${has_amd_3dnow_plus}")
message("has_ia64           = ${has_ia64}")    
message("has_snum           = ${has_snum}")    
message("has_proc_snum      = ${has_proc_snum}")    
message("proc_name          = ${proc_name}")    
message("proc_desc          = ${proc_desc}")    
message("os_name            = ${os_name}")    
message("os_rel             = ${os_rel}")    
message("os_ver             = ${os_ver}")    
message("os_platform        = ${os_platfor}")

message("PROJECT_SOURCE_DIR         [${PROJECT_SOURCE_DIR}]")
message("PROJECT_BINARY_DIR         [${PROJECT_BINARY_DIR}]")
message("CMAKE_SOURCE_DIR           [${CMAKE_SOURCE_DIR}]")
message("CMAKE_BINARY_DIR           [${CMAKE_BINARY_DIR}]")
message("CMAKE_CURRENT_SOURCE_DIR   [${CMAKE_CURRENT_SOURCE_DIR}]")
message("CMAKE_CURRENT_BINARY_DIR   [${CMAKE_CURRENT_BINARY_DIR}]")
message("CMAKE_BUILD_TYPE           [${CMAKE_BUILD_TYPE}]")

##### END-SECTION ##### Determine host environment  #####

##### SECTION ##### Setup version and last git commit hash #####

# Get date
string(TIMESTAMP year	%y) 
string(TIMESTAMP month	%m) 
string(TIMESTAMP timestamp	%Y-%m-%d-%H-%M-%s) 

set(VERSION_MAJOR ${year})
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

set(VERSION_BUILD_TIME ${timestamp})

#Find git, get last commit, save it in GIT_REVISION
find_package(Git)

if(GIT_FOUND)
	message("Found Git at:              [${GIT_EXECUTABLE}]")
		execute_process(
        	COMMAND ${GIT_EXECUTABLE} rev-list --max-count=1 HEAD
        	OUTPUT_VARIABLE GIT_REVISION
        	ERROR_QUIET
        )
	if(NOT ${GIT_REVISION} STREQUAL "")
        string(STRIP ${GIT_REVISION} GIT_REVISION)
    endif()
	message(STATUS "Current revision at: [${GIT_REVISION}]")
else()
	message(SEND_ERROR	"Git not found. Please install git")
endif()

# Convert the cmake/config.in.h to config.h
# while writing the version information in it.
configure_file(
	${PROJECT_SOURCE_DIR}/cmake/config.h.in
	${PROJECT_BINARY_DIR}/config.h
)

configure_file(
	${PROJECT_SOURCE_DIR}/ev/source/common/common/version_number.h.in
    ${PROJECT_SOURCE_DIR}/ev/source/common/common/version_number.h
)

##### END-SECTION ##### Setup version and last git commit hash #####

#find_program(CCACHE_FOUND ccache)
#if(CCACHE_FOUND)
#    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
#    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
#endif(CCACHE_FOUND)

##### SECTION ##### make distclean #####
set (fs-make-args FF_DPDK=${CMAKE_BINARY_DIR}/dpdk CMAKE_BUILD_CURDIR=${CMAKE_BINARY_DIR}/fs)

add_custom_target(fs-distclean
  COMMAND make -C ${CMAKE_SOURCE_DIR}/fs/lib/ ${fs-make-args} clean
  COMMAND make -C ${CMAKE_BINARY_DIR}/fs/ clean
  COMMAND "${CMAKE}" --build "${CMAKE_BINARY_DIR}" ..
  )

add_custom_target(fs-tools-distclean
  COMMAND make -C ${CMAKE_SOURCE_DIR}/fs/tools/ clean
  COMMAND "${CMAKE}" --build "${CMAKE_BINARY_DIR}" ..
  )

add_custom_target(dpdk-distclean
    COMMAND make -C ${CMAKE_SOURCE_DIR}/dpdk/ O=${CMAKE_BINARY_DIR}/dpdk clean
    COMMAND make -C ${CMAKE_BINARY_DIR}/dpdk/ clean
    COMMAND "${CMAKE}" --build "${CMAKE_BINARY_DIR}" ..
    )

set(ev_external_dir  ${CMAKE_SOURCE_DIR}/ev/external)

add_custom_target(distclean
  COMMAND "${CMAKE}" --build "${CMAKE_BINARY_DIR}" clean
  COMMAND make -C ${CMAKE_BINARY_DIR} clean
  DEPENDS fs-distclean dpdk-distclean openssl-distclean fs-tools-distclean
  COMMAND "${CMAKE}" --build "${CMAKE_BINARY_DIR}" ..
  COMMAND make -C ${ev_external_dir}/tclap/ clean
  COMMAND make -C ${ev_external_dir}/luajit-2.0/ clean
  COMMAND make -C ${ev_external_dir}/libevent/ clean
  COMMAND make -C ${ev_external_dir}/xxHash/ clean

)
##### END-SECTION ##### make distclean #####

##### SECTION ##### Build dependencies #####
add_subdirectory(dpdk)
add_subdirectory(ev/api)
# Compile boringssl early since others depend on it
add_subdirectory(ev/external/boringssl)
add_subdirectory(fs)

# Compile binaries in boringssli (see ev/external/boringssl/exes/CMakeLists.txt) since they depend on dpdk, fs
#add_subdirectory(ev/external/boringssl/exes)

add_subdirectory(ev)
##### END-SECTION ##### Build dependencies #####
