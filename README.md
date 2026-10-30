# Balatro for LG webOS (experimental)

Unofficial, open-source launcher for LG webOS TVs. **Bring your own legally obtained Balatro game files.**

> **Status: prototype only.** The current application is a first-run UI and remote-input smoke test. It does **not** yet import, install or play Balatro. There is no installable IPK release yet.

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

Open `app/index.html` in a desktop browser to inspect the prototype. This is not a runtime or compatibility test for LG TV hardware. The icon and production packaging must still be completed.

## Technical strategy

First test the MIT-licensed [Webmon](https://github.com/pustal/Webmon) native LÖVE 11.5 host using a minimal test game, then test a copy of Balatro provided by its owner. Its TV-side upload portal is a strong architectural reference. [SonicWebMania](https://github.com/pustal/SonicWebMania) demonstrates the desired local-PIN upload experience, but its restrictive license means we will **not** copy its implementation into this MIT project.

See [architecture and milestones](docs/ARCHITECTURE.md).

## Copyright and attribution

This repository must not ship Balatro source, music, art, game archives or DRM bypasses. Users are responsible for obtaining their own game copy. This project is not affiliated with LocalThunk, Playstack or LG. Before integrating third-party source code, review its full license and include attribution notices.

## Contributing

Compatibility testing, documentation and code contributions are welcome. Please do not attach proprietary game files to issues or pull requests. Never commit personal save files.

## License

Project-authored code is MIT-licensed. Third-party components, when introduced, may have separate conditions. See `LICENSE`.
