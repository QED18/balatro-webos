# Changelog

## 0.3.12 — 2026-10-31

- Fixed an overly aggressive one-second Magic Remote cursor sleep timeout. The launcher now keeps webOS's platform default, so pointer controls stay responsive during play.
- Clarified the safe exit behavior: fully exit the game before returning to webOS Home.

## 0.3.11 — 2026-10-30

First public beta of the native, game-file-free Balatro for webOS route.

- Added the private-LAN pairing portal and TV-visible transfer progress.
- Added macOS `.love` import and Windows/Proton `Balatro.exe` normalization.
- Added Magic Remote pointer support, TV-remote controller fallback, and Bluetooth-controller support.
- Added the webOS CRT compatibility overlay for the black U-shaped rendering artifact.
- Renamed the public app ID to `io.github.qed18.balatro`.
- Added plain-language setup, safety, architecture, portal, and runtime-licence documentation.
- Replaced the previous fan-made icon with an original project icon.

### Known limitations

- Sideloading requires LG Developer Mode; it is not an LG Content Store app.
- The pairing service is for a trusted home LAN only.
- The CRT workaround changes the original CRT-warp effect.
- Additional TV models and firmware versions still need compatibility testing.
