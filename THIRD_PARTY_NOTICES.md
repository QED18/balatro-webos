# Third-party notices

This repository contains no Balatro code, art, sound, music, archives, saves,
or other game files. The native smoke-test package contains only project-authored
source plus the open-source runtime dependencies listed below.

## Webmon integration reference

The native host's LÖVE start-up structure and webOS remote policy mapping were
adapted from [Webmon](https://github.com/pustal/Webmon), commit
`bb59edf545dd6ab3878dfc7441b20e9fffccf55d`, licensed under MIT.

Copyright (c) 2026 Webmon contributors. The full MIT license is in
`LICENSE-WEBMON` in a produced IPK and at the upstream repository. This project
does **not** include Webmon's upload server, game bridge, downloader, setup UI,
or any Pokémon-related code.

## Runtime dependencies in a produced smoke-test IPK

`native/scripts/build-webos.sh` reuses Webmon's pinned dependency recipe. A
package includes LÖVE 11.5 (zlib), LuaJIT at `c6ffc141` (MIT), SDL-webOS
2.30.12 (zlib), OpenAL Soft 1.23.1 (LGPL-2.1), FreeType 2.13.3 (FreeType
License), libogg/libvorbis/libtheora (BSD 3-Clause), libmodplug (public domain),
and the GCC runtime (GPL-3.0 with GCC Runtime Library Exception). Before any
release, copy the complete upstream notices for every distributed binary into
the package and review the static-linking obligations, particularly OpenAL Soft.

The host also follows LÖVE's `src/love.cpp` start-up model, licensed under zlib.
