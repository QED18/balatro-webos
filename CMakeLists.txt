# Game-file-free native LÖVE smoke test for LG webOS.
cmake_minimum_required(VERSION 3.21)
project(BalatroWebOSSmoke VERSION 0.2.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/native/cmake")

set(BALATRO_WEBOS_APP_ID "io.github.qed18.balatro.smoke" CACHE STRING "webOS application id")
set(BALATRO_WEBOS_WEBMON_SOURCE "" CACHE PATH "Checked-out Webmon source used only to build the MIT LÖVE host")
set(BALATRO_WEBOS_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/deps-work" CACHE PATH "Webmon dependency build directory")

if(NOT EXISTS "${BALATRO_WEBOS_WEBMON_SOURCE}/src/love/src/love.cpp")
    message(FATAL_ERROR "Set BALATRO_WEBOS_WEBMON_SOURCE to the pinned Webmon checkout (see native/scripts/build-webos.sh).")
endif()
set(BALATRO_WEBOS_DEPS_PREFIX "${BALATRO_WEBOS_DEPS_DIR}/prefix")
if(NOT EXISTS "${BALATRO_WEBOS_DEPS_PREFIX}/lib/liblove-11.5.so")
    message(FATAL_ERROR "LÖVE 11.5 dependencies missing in ${BALATRO_WEBOS_DEPS_DIR}; run native/scripts/build-webos.sh.")
endif()

add_library(BalatroSmoke::love SHARED IMPORTED)
set_target_properties(BalatroSmoke::love PROPERTIES
    IMPORTED_LOCATION "${BALATRO_WEBOS_DEPS_PREFIX}/lib/liblove-11.5.so"
    IMPORTED_SONAME "liblove-11.5.so"
    INTERFACE_INCLUDE_DIRECTORIES "${BALATRO_WEBOS_WEBMON_SOURCE}/src/love/src;${BALATRO_WEBOS_WEBMON_SOURCE}/src/love/src/modules;${BALATRO_WEBOS_DEPS_PREFIX}/include/luajit-2.1;${BALATRO_WEBOS_DEPS_PREFIX}/include/SDL2")
add_library(BalatroSmoke::luajit SHARED IMPORTED)
set_target_properties(BalatroSmoke::luajit PROPERTIES IMPORTED_LOCATION "${BALATRO_WEBOS_DEPS_PREFIX}/lib/libluajit-5.1.so.2")
add_library(BalatroSmoke::SDL2 SHARED IMPORTED)
set_target_properties(BalatroSmoke::SDL2 PROPERTIES IMPORTED_LOCATION "${BALATRO_WEBOS_DEPS_PREFIX}/lib/libSDL2-2.0.so.0")

add_executable(balatro-webos-smoke native/src/main.cpp native/src/Platform.cpp)
target_compile_features(balatro-webos-smoke PRIVATE cxx_std_17)
set_target_properties(balatro-webos-smoke PROPERTIES CXX_EXTENSIONS OFF BUILD_WITH_INSTALL_RPATH ON INSTALL_RPATH "\$ORIGIN/../lib")
target_include_directories(balatro-webos-smoke PRIVATE native/src)
target_compile_definitions(balatro-webos-smoke PRIVATE BALATRO_WEBOS_VERSION="${PROJECT_VERSION}")
target_compile_options(balatro-webos-smoke PRIVATE -Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion -Wno-psabi)
target_link_libraries(balatro-webos-smoke PRIVATE BalatroSmoke::love BalatroSmoke::luajit BalatroSmoke::SDL2)

set(BALATRO_SMOKE_LOVE "${CMAKE_CURRENT_BINARY_DIR}/smoke.love")
add_custom_command(OUTPUT "${BALATRO_SMOKE_LOVE}"
    COMMAND "${CMAKE_COMMAND}" -E tar cf "${BALATRO_SMOKE_LOVE}" --format=zip main.lua
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/native/sample"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/native/sample/main.lua"
    COMMENT "Packing the original no-assets smoke sample")
add_custom_target(smoke-love DEPENDS "${BALATRO_SMOKE_LOVE}")

include(WebOSPackage)
