find_program(BALATRO_ARES_PACKAGE ares-package)
set(BALATRO_IPK_STAGE "${CMAKE_CURRENT_BINARY_DIR}/ipk")
set(BALATRO_DIST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist")
set(BALATRO_IPK_NAME "balatro-for-webos-v${PROJECT_VERSION}_arm.ipk")
get_filename_component(_compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
set(_runtime_dir "${_compiler_dir}/../arm-webos-linux-gnueabi/lib")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/native/webos/appinfo.json.in" "${CMAKE_CURRENT_BINARY_DIR}/appinfo.json" @ONLY)
if(NOT CMAKE_STRIP)
  set(CMAKE_STRIP strip)
endif()
add_custom_target(webos-stage
  COMMAND "${CMAKE_COMMAND}" -E rm -rf "${BALATRO_IPK_STAGE}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${BALATRO_IPK_STAGE}/bin" "${BALATRO_IPK_STAGE}/lib" "${BALATRO_IPK_STAGE}/share"
  COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/appinfo.json" "${BALATRO_IPK_STAGE}/"
  COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/app/icon.png" "${CMAKE_CURRENT_SOURCE_DIR}/app/largeIcon.png" "${BALATRO_IPK_STAGE}/"
  COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:balatro-webos-smoke>" "${BALATRO_IPK_STAGE}/bin/balatro-webos-smoke"
  COMMAND "${CMAKE_STRIP}" --strip-unneeded "${BALATRO_IPK_STAGE}/bin/balatro-webos-smoke"
  COMMAND "${CMAKE_COMMAND}" -E copy "${BALATRO_SMOKE_LOVE}" "${BALATRO_IPK_STAGE}/share/smoke.love"
  COMMAND "${CMAKE_COMMAND}" -E copy "${BALATRO_WEBOS_DEPS_PREFIX}/lib/liblove-11.5.so" "${BALATRO_WEBOS_DEPS_PREFIX}/lib/libluajit-5.1.so.2" "${BALATRO_WEBOS_DEPS_PREFIX}/lib/libSDL2-2.0.so.0" "${_runtime_dir}/libstdc++.so.6" "${_runtime_dir}/libgcc_s.so.1" "${BALATRO_IPK_STAGE}/lib/"
  COMMAND "${CMAKE_STRIP}" --strip-unneeded "${BALATRO_IPK_STAGE}/lib/liblove-11.5.so" "${BALATRO_IPK_STAGE}/lib/libluajit-5.1.so.2" "${BALATRO_IPK_STAGE}/lib/libSDL2-2.0.so.0" "${BALATRO_IPK_STAGE}/lib/libstdc++.so.6" "${BALATRO_IPK_STAGE}/lib/libgcc_s.so.1"
  COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md" "${CMAKE_CURRENT_SOURCE_DIR}/native/LICENSE-WEBMON" "${BALATRO_IPK_STAGE}/"
  COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/third_party_licenses" "${BALATRO_IPK_STAGE}/third_party_licenses"
  DEPENDS balatro-webos-smoke smoke-love
  VERBATIM)
if(BALATRO_ARES_PACKAGE)
  add_custom_target(webos-package
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${BALATRO_DIST_DIR}"
    COMMAND "${BALATRO_ARES_PACKAGE}" --no-minify "${BALATRO_IPK_STAGE}" -o "${BALATRO_DIST_DIR}"
    DEPENDS webos-stage VERBATIM)
else()
  add_custom_target(webos-package COMMAND "${CMAKE_COMMAND}" -E echo "ares-package not found: npm install -g @webos-tools/cli" COMMAND "${CMAKE_COMMAND}" -E false DEPENDS webos-stage)
endif()
