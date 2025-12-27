cmake_minimum_required(VERSION 3.16)
project(gz_console_thingy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_GENERATOR_PLATFORM x64)

# global defines
add_definitions(-DWIN32 -DWIN32_LEAN_AND_MEAN)

# configuration-specific settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG -D_DEBUG -D_ITERATOR_DEBUG_LEVEL=0)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
endif()

# set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

#
# asmjit library
#
add_library(asmjit STATIC)
target_compile_definitions(asmjit PRIVATE ASMJIT_STATIC)
target_include_directories(asmjit PRIVATE deps/meow-hook/third_party/asmjit/src)
file(GLOB_RECURSE ASMJIT_SOURCES "deps/meow-hook/third_party/asmjit/src/*.cpp" "deps/meow-hook/third_party/asmjit/src/*.h")
target_sources(asmjit PRIVATE ${ASMJIT_SOURCES})

#
# zydis library
#
add_library(zydis STATIC)
target_compile_definitions(zydis PRIVATE
    ZYCORE_STATIC_DEFINE
    ZYDIS_STATIC_DEFINE
)
target_include_directories(zydis PRIVATE
    deps/meow-hook/third_party/zydis/src
    deps/meow-hook/third_party/zydis/include
    deps/meow-hook/third_party/zydis/msvc
    deps/meow-hook/third_party/zydis/dependencies/zycore/include
)
file(GLOB_RECURSE ZYDIS_SOURCES
    "deps/meow-hook/third_party/zydis/dependencies/zycore/src/*.c"
    "deps/meow-hook/third_party/zydis/dependencies/zycore/src/*.h"
    "deps/meow-hook/third_party/zydis/dependencies/zycore/include/*.h"
    "deps/meow-hook/third_party/zydis/src/*.c"
    "deps/meow-hook/third_party/zydis/src/*.h"
    "deps/meow-hook/third_party/zydis/include/Zydis/*.h"
)
target_sources(zydis PRIVATE ${ZYDIS_SOURCES})

#
# fw1 library
#
add_library(fw1 STATIC)
file(GLOB FW1_SOURCES "deps/fw1/*.h" "deps/fw1/*.cpp")
target_sources(fw1 PRIVATE ${FW1_SOURCES})

#
# imgui library
#
add_subdirectory(deps/imgui-cmake)

#
# meow-hook library
#
add_library(meow-hook STATIC)
target_compile_definitions(meow-hook PRIVATE
    NOMINMAX
    ASMJIT_STATIC
    ZYCORE_STATIC_DEFINE
    ZYDIS_STATIC_DEFINE
)
target_include_directories(meow-hook PRIVATE
    deps/meow-hook/include
    deps/meow-hook/third_party/asmjit/src
    deps/meow-hook/third_party/zydis/src
    deps/meow-hook/third_party/zydis/include
    deps/meow-hook/third_party/zydis/msvc
    deps/meow-hook/third_party/zydis/dependencies/zycore/include
)
file(GLOB MEOW_HOOK_SOURCES
    "deps/meow-hook/src/*.cc"
    "deps/meow-hook/src/*.h"
    "deps/meow-hook/include/meow_hook/*.h"
)
target_sources(meow-hook PRIVATE ${MEOW_HOOK_SOURCES})
target_link_libraries(meow-hook PRIVATE asmjit zydis)

#
# xinput9_1_0 main project (DLL)
#
add_library(xinput9_1_0 SHARED)
target_compile_definitions(xinput9_1_0 PRIVATE NOMINMAX)
target_include_directories(xinput9_1_0 PRIVATE
    src
    deps/fw1
    deps/meow-hook/include
)

# source files (excluding generator)
file(GLOB_RECURSE XINPUT_SOURCES
    "src/*.h"
    "src/*.cpp"
)
# remove generator files
file(GLOB_RECURSE GENERATOR_SOURCES "src/generator/*")
list(REMOVE_ITEM XINPUT_SOURCES ${GENERATOR_SOURCES})

target_sources(xinput9_1_0 PRIVATE ${XINPUT_SOURCES} src/xinput9_1_0.def)
target_link_libraries(xinput9_1_0 PRIVATE fw1 meow-hook imgui)

# set the DEF file (MSVC only)
if(MSVC)
    set_target_properties(xinput9_1_0 PROPERTIES
        LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/src/xinput9_1_0.def"
    )
else()
    # For MinGW/GCC, use different approach for DEF files
    set_target_properties(xinput9_1_0 PROPERTIES
        LINK_FLAGS "-Wl,--def,${CMAKE_CURRENT_SOURCE_DIR}/src/xinput9_1_0.def"
    )
endif()

# post-build command to copy DLL
add_custom_command(TARGET xinput9_1_0 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:xinput9_1_0>
    "C:/Program Files (x86)/Steam/steamapps/common/GenerationZero/xinput9_1_0.dll"
    COMMENT "Copying xinput9_1_0.dll to Generation Zero directory"
)

#
# address-generator console app
#
add_executable(address-generator)
file(GLOB_RECURSE GENERATOR_SOURCES "src/generator/*")
target_sources(address-generator PRIVATE ${GENERATOR_SOURCES})
target_include_directories(address-generator PRIVATE deps/meow-hook/include)
target_link_libraries(address-generator PRIVATE meow-hook)

# set startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT xinput9_1_0)
