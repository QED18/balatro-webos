# Balatro for LG webOS (experimental)

Unofficial, open-source launcher for LG webOS TVs. **Bring your own legally obtained Balatro game files.**

> **Status: prototype only.** The current browser app is a first-run UI and remote-input diagnostic. A separate, game-file-free *native LÖVE runtime smoke test* is now source-complete but **has not been cross-built or run on a TV**. Neither component imports, installs, or plays Balatro. There is no installable release yet.

## Goal

1. Install a small, game-file-free app via Homebrew Channel.
2. Open it on the TV and see an address plus temporary PIN.
3. From a PC on the same private network, provide your purchased Steam copy.
4. Transfer game files locally to the TV and play offline with the LG Magic Remote.

These are planned capabilities, not shipped features.

## Current prototype

- `app/index.html`: fullscreen first-run UI.
- `app/js/app.js`: webOS key diagnostics and OK-button test.
- `app/style.css`: TV-first responsive layout.
- `app/appinfo.json`: preliminary webOS package metadata.

Open `app/index.html` in a desktop browser to inspect the prototype. This is not a runtime or compatibility test for LG TV hardware.

## Native runtime smoke test (not Balatro support)

`native/` contains an original LÖVE 11.5 sample and a narrow native host patterned after the MIT-licensed Webmon integration. It runs **only** the bundled `smoke.love` sample: it renders text/shapes, requests a generated 440 Hz tone, logs D-pad/OK/Back input, and writes a counter save. It has no network listener, uploader, game-file discovery, downloader, root command, or game content.

On a Linux x86_64 build host with the webOSbrew Buildroot SDK, `ares-package`, and Webmon's build prerequisites:

```sh
export WEBOS_SDK=/absolute/path/to/arm-webos-linux-gnueabi_sdk-buildroot
native/scripts/build-webos.sh
```

The script pins Webmon to `bb59edf545dd6ab3878dfc7441b20e9fffccf55d`, reuses its LÖVE dependency recipe, and asks `ares-package` to make `dist/io.github.qed18.balatro.smoke_0.2.0_arm.ipk`. It has not been executed in this repository because the SDK, 32-bit LuaJIT host build support, and `ares-package` are not present. Run `python3 scripts/verify-source.py` for the source-level checks available without the SDK.

The package source uses the original 512×512 `app/icon.png` and `app/largeIcon.png`; it includes `LICENSE`, `LICENSE-WEBMON`, and `THIRD_PARTY_NOTICES.md`. Complete the notices and LGPL distribution review before publishing any binary.

## Technical strategy

First test the MIT-licensed [Webmon](https://github.com/pustal/Webmon) native LÖVE 11.5 host using a minimal test game, then test a copy of Balatro provided by its owner. Its TV-side upload portal is a strong architectural reference. [SonicWebMania](https://github.com/pustal/SonicWebMania) demonstrates the desired local-PIN upload experience, but its restrictive license means we will **not** copy its implementation into this MIT project.

See [architecture and milestones](docs/ARCHITECTURE.md).

## Copyright and attribution

This repository must not ship Balatro source, music, art, game archives or DRM bypasses. Users are responsible for obtaining their own game copy. This project is not affiliated with LocalThunk, Playstack or LG. Before integrating third-party source code, review its full license and include attribution notices.

## Contributing

Compatibility testing, documentation and code contributions are welcome. Please do not attach proprietary game files to issues or pull requests. Never commit personal save files.

## License

Project-authored code is MIT-licensed. Third-party components, when introduced, may have separate conditions. See `LICENSE`.
