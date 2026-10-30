#!/usr/bin/env bash
# Build the game-file-free ARM IPK on Linux x86_64. No Balatro files are read,
# copied, uploaded, or produced by this command.
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
: "${WEBOS_SDK:?Set WEBOS_SDK to the unpacked arm-webos-linux-gnueabi_sdk-buildroot SDK}"
webmon_commit="bb59edf545dd6ab3878dfc7441b20e9fffccf55d"
webmon_source="${BALATRO_WEBOS_WEBMON_SOURCE:-${project_root}/third_party/webmon}"
deps="${BALATRO_WEBOS_DEPS_DIR:-${project_root}/third_party/deps-work}"

if [[ "$(uname -s)" != "Linux" || "$(uname -m)" != "x86_64" ]]; then
  echo "This build recipe is supported only on Linux x86_64." >&2
  exit 2
fi
# The Buildroot SDK release ships two libstdc++.la files with its CI path in
# `libdir`. Rewrite only that relocatable metadata to this unpacked SDK.
for sdk_libtool_archive in \
  "${WEBOS_SDK}/arm-webos-linux-gnueabi/lib/libstdc++.la" \
  "${WEBOS_SDK}/arm-webos-linux-gnueabi/sysroot/usr/lib/libstdc++.la"; do
  if [[ -f "${sdk_libtool_archive}" ]]; then
    sdk_libdir="$(dirname "${sdk_libtool_archive}")"
    sed -i "s|^libdir=.*|libdir='${sdk_libdir}'|" "${sdk_libtool_archive}"
  fi
done
if [[ ! -d "${webmon_source}/.git" ]]; then
  mkdir -p "$(dirname "${webmon_source}")"
  git clone --no-checkout https://github.com/pustal/Webmon.git "${webmon_source}"
fi
git -C "${webmon_source}" fetch --depth 1 origin "${webmon_commit}"
git -C "${webmon_source}" checkout --detach --force FETCH_HEAD
[[ "$(git -C "${webmon_source}" rev-parse HEAD)" == "${webmon_commit}" ]]

# This is Webmon's pinned LÖVE 11.5 dependency recipe. Review its notices
# before distributing an IPK; this project does not vendor Webmon sources.
# OpenAL Soft 1.23.1 predates CMake 3.5 policy retirement and needs C++17.
# These build-host compatibility settings reach both its CMake policy layer and
# compiler flags. OpenAL 1.23.1 hard-codes C++14 and relies on indirect
# fixed-width type includes, so fix those fetched build-dependency assumptions
# before compiling it; the IPK contains no source files.
deps_script="${webmon_source}/scripts/build-deps.sh"
grep -q -- '-DALSOFT_UTILS=OFF' "${deps_script}"
sed -i 's/-DALSOFT_UTILS=OFF/-DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_CXX_STANDARD=17 -DALSOFT_UTILS=OFF/' "${deps_script}"
sed -i 's/-DCMAKE_CXX_FLAGS="${CXXFLAGS}" -DLIBTYPE=STATIC/-DCMAKE_CXX_FLAGS="${CXXFLAGS} -std=gnu++17 -include stdint.h" -DLIBTYPE=STATIC/' "${deps_script}"
sed -i '/fetch_git "${OPENAL_REPO}" "${OPENAL_TAG}" openal/a\    sed -i '\''s/CXX_STANDARD 14/CXX_STANDARD 17/'\'' "${src}/openal/CMakeLists.txt"' "${deps_script}"
sed -i 's/ -static-libstdc++ -static-libgcc//' "${deps_script}"
"${deps_script}" "${deps}"

cmake -S "${project_root}" -B "${project_root}/build-webos" \
  -DCMAKE_TOOLCHAIN_FILE="${WEBOS_SDK}/share/buildroot/toolchainfile.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBALATRO_WEBOS_WEBMON_SOURCE="${webmon_source}" \
  -DBALATRO_WEBOS_DEPS_DIR="${deps}"
cmake --build "${project_root}/build-webos" --parallel
cmake --build "${project_root}/build-webos" --target webos-package
