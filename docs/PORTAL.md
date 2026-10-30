# Pairing portal

The first-run portal is part of the native TV app. It is not a hosted website and it does not upload game files to GitHub or any cloud service.

## Owner flow

1. With no owner archive present, the TV app starts a small HTTP server on port `47811` through `47821` and shows a local URL plus a new four-digit PIN.
2. A computer on the same home network opens that URL, enters the PIN, chooses a platform, and selects the game file from its own installed Steam copy.
3. The browser streams the selected file directly to the TV. Both the browser and the TV setup screen show progress.
4. The TV validates and stages the archive, then reports completion. The owner quits and reopens the app to launch the imported game.

The portal is deliberately Balatro-specific. It accepts only a file named `Balatro.love` or `Balatro.exe`.

## Accepted inputs

| Computer | File chosen in the portal | What the TV stores |
| --- | --- | --- |
| macOS | `Balatro.app/Contents/Resources/Balatro.love` | The selected archive |
| Windows | `Balatro.exe` in the Steam game directory | The LÖVE archive embedded in the executable |
| Linux / Steam Deck | `Balatro.exe` when a Proton-style install provides it | The embedded LÖVE archive |

The macOS `.app` package itself cannot be selected and unpacked by a normal browser file picker. The owner must select the `.love` file inside its `Resources` directory. This keeps the setup dependency-free on the computer.

## Validation

The native server enforces a 160 MB maximum request, writes the request into private staging storage, and validates the ZIP end-of-central-directory record before activation. It rejects archives with unsafe paths, missing `main.lua` at the root, malformed central directories, or an invalid embedded LÖVE archive. A Windows executable is normalized into a standalone `.love` archive before it replaces the previous owner archive.

The app writes a small status file atomically while receiving, verifying, rejecting, and completing an import. The first-run LÖVE screen reads it to show the same progress on the TV.

## Privacy and threat model

The pairing service exists only while the first-run setup screen is open. It uses a new PIN each time that app process starts, and the UI advertises a private IPv4 address when one is available.

This is meant for a trusted home LAN. It is **not** an Internet-facing service: there is no TLS, account system, authentication beyond the short PIN, or failed-PIN rate limit. Do not port-forward its port, use it on an untrusted network, or share its address publicly.

## Data locations

The imported archive, temporary staging files, save data, and portal status are app-private TV data. None is committed to the repository, added to a release asset, or sent to analytics. Updates are designed to leave that private data alone, but owners should keep normal backups of their saves.

## No QR code yet

The portal currently shows a local address and PIN. A QR-code convenience flow is not implemented; it must not be documented or advertised until it is real.
