# Architecture

## Design boundary

This repository distributes a native webOS host and open-source runtime components only. It never contains Balatro game data. A game archive appears only after an owner deliberately transfers it over their private home network into their TV's private app storage.

## Runtime flow

```text
first launch without game
  → native host starts portal + original setup .love screen
  → owner browser sends a PIN-protected local upload
  → native host validates/stages the archive
  → owner closes and reopens app
  → native host starts the owner-supplied Balatro archive
```

`native/src/main.cpp` decides which route to use. `PortalServer` runs only when `game/Balatro.love` is absent. When an owner archive exists, the host installs a project-authored shader override for the webOS CRT rendering issue, configures the remote input layer, and starts LÖVE 11.5 with the archive.

## Components

- `native/src/main.cpp` — LÖVE bootstrap and first-run/game decision.
- `native/src/PortalServer.*` — local pairing HTTP server, file staging, ZIP validation, and Windows-executable normalization.
- `native/src/Platform.*` — private data directory, CRT workaround, and webOS remote-to-virtual-controller mapping.
- `native/sample/` — original game-file-free setup screen shown before an owner import.
- `native/cmake/WebOSPackage.cmake` — native IPK staging and packaging.
- `app/` — static visual prototype only; it is not the native runtime portal.

## Tested compatibility

The current native route has been tested on an LG webOS 9.2 TV with an owner-supplied archive. The portal accepted a macOS `.love` archive and a Windows `Balatro.exe`; the imported game launched. Magic Remote pointer input and a Bluetooth DualSense controller worked. The D-pad/OK virtual-controller path remains a compatibility fallback and should be tested on additional TV firmware before relying on it.

## Controls

The platform layer creates an SDL virtual controller when the TV's SDL build permits it. Arrows become the D-pad, Return/Green becomes A, and Back/Red becomes B. It suppresses the corresponding keyboard event only after emitting the virtual controller event, avoiding the game's mixed pointer/controller mode. Bluetooth controllers bypass that emulation and are preferred.

## Security constraints

- Import only on a trusted private network.
- Keep the TV's Developer Mode SSH/key access private; never expose it to the Internet.
- Do not commit or attach owner archives, saves, or error logs that reveal private paths.
- Treat the local PIN as pairing convenience, not a strong authentication system.

## Validation before broader releases

1. Test the release IPK as a clean install on another TV model and webOS version.
2. Confirm a fresh Mac and Windows/Proton transfer, relaunch, save, and controller session.
3. Record the IPK SHA-256, TV model, webOS version, and any known rendering/input issue in the release notes.
4. Review runtime dependency licences whenever the pinned build recipe changes.
