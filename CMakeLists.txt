cmake_minimum_required(VERSION 3.16)
project(gz_console_thingy VERSION 1.5.0)

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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/version.h @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/resource.rc.in ${CMAKE_CURRENT_BINARY_DIR}/generated/resource.rc @ONLY)

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
# minhook library
#
add_subdirectory(deps/minhook)

#
# meow-hook library
#
add_library(meow-hook STATIC)
target_include_directories(meow-hook PRIVATE
    deps/meow-hook/include
)
target_compile_definitions(meow-hook PRIVATE
    NOMINMAX
)
file(GLOB MEOW_HOOK_SOURCES
    "deps/meow-hook/src/pattern_search.cc"
    "deps/meow-hook/src/instruction_set.cc"
    "deps/meow-hook/src/instruction_set.h"
    "deps/meow-hook/include/meow_hook/pattern_search.h"
    "deps/meow-hook/include/meow_hook/util.h"
)
target_sources(meow-hook PRIVATE ${MEOW_HOOK_SOURCES})

#
# xinput9_1_0 main project (DLL)
#
add_library(xinput9_1_0 SHARED)
target_compile_definitions(xinput9_1_0 PRIVATE NOMINMAX)
target_include_directories(xinput9_1_0 PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
    src
    deps/fw1
    deps/minhook/include
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

target_sources(xinput9_1_0 PRIVATE ${XINPUT_SOURCES} src/xinput9_1_0.def ${CMAKE_CURRENT_BINARY_DIR}/generated/resource.rc)
target_link_libraries(xinput9_1_0 PRIVATE fw1 minhook imgui)

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
