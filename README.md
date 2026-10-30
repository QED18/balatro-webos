# Balatro for LG webOS (experimental)

Unofficial, open-source launcher for LG webOS TVs. **Bring your own legally obtained Balatro game files.**

> **Status: prototype only.** The supported proof-of-concept route is [Webmon](https://github.com/pustal/Webmon): its LÖVE 11.5 runtime has run this repository's original, no-assets smoke test on an LG webOS TV. The custom native IPK remains experimental and is not a release or a prerequisite for the Webmon path. Neither route imports, installs, or plays Balatro.

## Goal

1. Install Webmon via Homebrew Channel and verify the TV's LÖVE runtime with this repository's original smoke test.
2. Keep all source, CI artifacts, and releases game-file-free.
3. Only after compatibility work, design a separate opt-in path for a user's own lawfully obtained game files.

These are planned capabilities, not shipped features.

## Current prototype

- `app/index.html`: fullscreen first-run UI.
- `app/js/app.js`: webOS key diagnostics and OK-button test.
- `app/style.css`: TV-first responsive layout.
- `app/appinfo.json`: preliminary webOS package metadata.

Open `app/index.html` in a desktop browser to inspect the prototype. This is not a runtime or compatibility test for LG TV hardware.

## Supported smoke test: Webmon (not Balatro support)

Webmon is the fastest validated path to test a LÖVE program on a compatible webOS TV. Install Webmon through Homebrew Channel, open it on the TV, press remote **1** to display its local upload address and temporary PIN, then upload the original `.love` smoke archive from a device on the same private network. Exit and reopen Webmon to run the newest uploaded `.love` file.

The smoke program renders original shapes/text, requests a generated 440 Hz tone, logs D-pad/OK/Back input, and writes a counter save. It contains no Balatro code, art, audio, archives, save data, networking, or downloader.

The repository's `native/sample/main.lua` is the source for that test. Package it with `main.lua` at the archive root:

```sh
(cd native/sample && cmake -E tar cf ../../balatro-webos-smoke.love --format=zip main.lua)
```

## Experimental native IPK build

`native/` contains an original LÖVE 11.5 sample and a narrow native host patterned after the MIT-licensed Webmon integration. It runs **only** the bundled `smoke.love` sample: it renders text/shapes, requests a generated 440 Hz tone, logs D-pad/OK/Back input, and writes a counter save. It has no network listener, uploader, game-file discovery, downloader, root command, or game content.

On a Linux x86_64 build host with the webOSbrew Buildroot SDK, `ares-package`, and Webmon's build prerequisites:

```sh
export WEBOS_SDK=/absolute/path/to/arm-webos-linux-gnueabi_sdk-buildroot
native/scripts/build-webos.sh
```

The script pins Webmon to `bb59edf545dd6ab3878dfc7441b20e9fffccf55d`, reuses its LÖVE dependency recipe, and asks `ares-package` to make `dist/io.github.qed18.balatro.smoke_0.2.0_arm.ipk`. This path is experimental: do not treat it as a supported install method or publish its output until it has a clean cross-build and independent TV validation. Run `python3 scripts/verify-source.py` for the source-level checks available without the SDK.

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
