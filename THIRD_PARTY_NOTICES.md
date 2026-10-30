# Third-party notices

This repository and its release IPKs contain **no Balatro game code, art, sound, archive, save, downloader, or game-file service**. The native host packages the following open-source runtime components. Their complete licence texts are included in a release IPK under `third_party_licenses/`.

| Component | Licence | Included notice |
| --- | --- | --- |
| Webmon integration reference | MIT | `native/LICENSE-WEBMON` |
| LÖVE 11.5 | zlib | `third_party_licenses/LOVE-11.5-zlib.txt` |
| LuaJIT | MIT | `third_party_licenses/LuaJIT-MIT.txt` |
| SDL-webOS 2.30.12 | zlib | `third_party_licenses/SDL2-zlib.txt` |
| OpenAL Soft 1.23.1 | LGPL-2.1 | `third_party_licenses/OpenAL-Soft-LGPL-2.1.txt` |
| FreeType 2.13.3 | FreeType License | `third_party_licenses/FreeType.txt` |
| libogg | BSD 3-Clause | `third_party_licenses/libogg-BSD-3-Clause.txt` |
| libvorbis | BSD 3-Clause | `third_party_licenses/libvorbis-BSD-3-Clause.txt` |
| libtheora | BSD 3-Clause | `third_party_licenses/libtheora-BSD-3-Clause.txt` |
| libmodplug | public domain | `third_party_licenses/libmodplug-public-domain.txt` |
| GCC runtime libraries | GPL-3.0 with GCC Runtime Library Exception 3.1 | `third_party_licenses/GPL-3.0.txt`, `third_party_licenses/GCC-Runtime-Library-Exception-3.1.txt` |

## Webmon reference and attribution

The LÖVE startup structure and webOS remote policy mapping were adapted from [Webmon](https://github.com/pustal/Webmon), commit `bb59edf545dd6ab3878dfc7441b20e9fffccf55d`, under the MIT licence. This project has its own native portal and contains no Webmon game bridge, downloader, setup assets, or Pokémon-related code.

## Runtime source and rebuildability

`native/scripts/build-webos.sh` fetches the pinned Webmon dependency recipe and builds the runtime libraries locally. The full build recipe is included so recipients can replace or rebuild the LGPL component. The release package dynamically links its included runtime libraries; review the licences above before redistributing a modified package.

## Project assets

The app's playing-card icon is project-authored, original pixel art generated for this project. It contains no Balatro artwork, logo, character, or other third-party game asset.

## Project licence

Project-authored source is MIT-licensed under [`LICENSE`](LICENSE). Third-party components keep their own licences and notices.
