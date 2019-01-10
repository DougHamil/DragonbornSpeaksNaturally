cmake_minimum_required(VERSION 3.5)

project(dsn_plugin)


###############
# Options
###############

# set C++ standard
# don't need at current, keep it for future
#set(CMAKE_CXX_STANDARD 17)

#
# Static linking VC++ runtime library
#
macro(set_linking_vclib CompilerFlag LinkFlag)
    string(REPLACE "/MDd" "" ${CompilerFlag} "${${CompilerFlag}}")
    string(REPLACE "/MD" "" ${CompilerFlag} "${${CompilerFlag}}")
    string(REPLACE "/MTd" "" ${CompilerFlag} "${${CompilerFlag}}")
    string(REPLACE "/MT" "" ${CompilerFlag} "${${CompilerFlag}}")
    set(${CompilerFlag} "${${CompilerFlag}} ${LinkFlag}")
    message("${CompilerFlag}=${${CompilerFlag}}")
endmacro()

option(STATIC_VCLIB "Linking static VC++ runtime library (/MT or /MTd)." ON)

if(STATIC_VCLIB)
    message("-- Linking static VC++ runtime library (/MT or /MTd): -DSTATIC_VCLIB=ON")

    set_linking_vclib(CMAKE_CXX_FLAGS_DEBUG          "/MTd")
    set_linking_vclib(CMAKE_C_FLAGS_DEBUG            "/MTd")
    set_linking_vclib(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MTd")
    set_linking_vclib(CMAKE_C_FLAGS_RELWITHDEBINFO   "/MTd")
    set_linking_vclib(CMAKE_CXX_FLAGS_RELEASE        "/MT")
    set_linking_vclib(CMAKE_C_FLAGS_RELEASE          "/MT")
    set_linking_vclib(CMAKE_CXX_FLAGS_MINSIZEREL     "/MT")
    set_linking_vclib(CMAKE_C_FLAGS_MINSIZEREL       "/MT")
else()
    message("-- Linking dynamic VC++ runtime library (/MD or /MDd): -DSTATIC_VCLIB=OFF")

    set_linking_vclib(CMAKE_CXX_FLAGS_DEBUG          "/MDd")
    set_linking_vclib(CMAKE_C_FLAGS_DEBUG            "/MDd")
    set_linking_vclib(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MDd")
    set_linking_vclib(CMAKE_C_FLAGS_RELWITHDEBINFO   "/MDd")
    set_linking_vclib(CMAKE_CXX_FLAGS_RELEASE        "/MD")
    set_linking_vclib(CMAKE_C_FLAGS_RELEASE          "/MD")
    set_linking_vclib(CMAKE_CXX_FLAGS_MINSIZEREL     "/MD")
    set_linking_vclib(CMAKE_C_FLAGS_MINSIZEREL       "/MD")
endif()

#
# Compile with multiple processors
#
option(MP "Enable multiprocessor compilation." ON)

if(MP)
    add_definitions(/MP)
    message("-- Multiprocessor compilation enabled (/MP): -DMP=ON")
else()
    message("-- Multiprocessor compilation disabled (without /MP): -DMP=OFF")
endif()

#
# Disable SAFESEH
#
option(SAFESEH "Enable SAFESEH (/SAFESEH)." OFF)

if(SAFESEH)
    message("-- SAFESEH enabled (/SAFESEH): -DSAFESEH=ON")
else()
    message("-- SAFESEH disabled (/SAFESEH:NO): -DSAFESEH=OFF")
endif()

macro(set_target_safeseh target)
    if(SAFESEH)
        target_link_options(${target} PRIVATE /SAFESEH)
    else()
        target_link_options(${target} PRIVATE /SAFESEH:NO)
    endif()
endmacro()


###############
# Source codes and Targets
###############

#
# SKSE VR for SkyrimVR
#

file(GLOB SVR_COMMON_SRC        svr/common/*.h        svr/common/*.cpp)
file(GLOB SVR_SKSE64_COMMON_SRC svr/skse64_common/*.h svr/skse64_common/*.cpp)
file(GLOB SVR_SKSE64_SRC        svr/skse64/*.h        svr/skse64/*.cpp)

add_library(svr_common        STATIC ${SVR_COMMON_SRC})
add_library(svr_skse64_common STATIC ${SVR_SKSE64_COMMON_SRC})
add_library(svr_skse64        STATIC ${SVR_SKSE64_SRC})

target_include_directories(svr_common PUBLIC svr/ svr/xbyak/)
target_compile_options(svr_common PUBLIC /FI"common/IPrefix.h")
target_compile_definitions(svr_skse64 PRIVATE
    SKSE64_EXPORTS
    RUNTIME
    RUNTIME_VERSION=0x01030401
    IS_VR
)

target_link_libraries(svr_skse64_common svr_common)
target_link_libraries(svr_skse64 svr_skse64_common)

#
# SKSE64 for SkyrimSE
#

file(GLOB SSE_COMMON_SRC        sse/common/*.h        sse/common/*.cpp)
file(GLOB SSE_SKSE64_COMMON_SRC sse/skse64_common/*.h sse/skse64_common/*.cpp)
file(GLOB SSE_SKSE64_SRC        sse/skse64/*.h        sse/skse64/*.cpp)

add_library(sse_common        STATIC ${SSE_COMMON_SRC})
add_library(sse_skse64_common STATIC ${SSE_SKSE64_COMMON_SRC})
add_library(sse_skse64        STATIC ${SSE_SKSE64_SRC})

target_include_directories(sse_common PUBLIC sse/ sse/xbyak/)
target_compile_options(sse_common PUBLIC /FI"common/IPrefix.h")
target_compile_definitions(sse_skse64 PRIVATE
    SKSE64_EXPORTS
    RUNTIME
    RUNTIME_VERSION=0x01030401
)

target_link_libraries(sse_skse64_common sse_common)
target_link_libraries(sse_skse64 sse_skse64_common)

#
# dsn plugin
#
file(GLOB_RECURSE DSN_PLUGIN_SRC
    dsn_plugin/*.h
    dsn_plugin/*.hpp
    dsn_plugin/*.cpp
    dsn_plugin/*.def
    dsn_plugin/*.txt
    CMakeLists.txt
)

# for SkyrimVR
add_library(dsn_plugin_vr SHARED ${DSN_PLUGIN_SRC})
target_link_libraries(dsn_plugin_vr svr_skse64 version)
set_target_safeseh(dsn_plugin_vr)

target_include_directories(dsn_plugin_vr PUBLIC dsn_plugin/)
set_target_properties(dsn_plugin_vr PROPERTIES
  OUTPUT_NAME "dragonborn_speaks_naturally"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/SkyrimVR"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/SkyrimVR"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/SkyrimVR"
)

# for SkyrimSE
add_library(dsn_plugin_se SHARED ${DSN_PLUGIN_SRC})
target_link_libraries(dsn_plugin_se sse_skse64 version)
set_target_safeseh(dsn_plugin_se)

target_include_directories(dsn_plugin_se PUBLIC dsn_plugin/)
set_target_properties(dsn_plugin_se PROPERTIES
  OUTPUT_NAME "dragonborn_speaks_naturally"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/SkyrimSE"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/SkyrimSE"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/SkyrimSE"
)
