# Architecture and validation gates

## User journey (target)
Install launcher from Homebrew Channel → see local pairing URL and one-time PIN → select legally obtained Balatro game archive on a PC on the same LAN → validate and transfer → store privately on the TV → run offline using Magic Remote.

## Implementation status
- **Implemented:** setup screen, keyboard/remote event diagnostics.
- **Not yet implemented:** network server, pairing/PIN, upload, extraction, storage management, native LÖVE integration, audio/gameplay, save support, packaging CI, Homebrew submission.

## Candidate foundations
- [Webmon](https://github.com/pustal/Webmon): MIT, webOS-native LÖVE 11.5 runtime; read license and bundled notices before reuse.
- [SonicWebMania](https://github.com/pustal/SonicWebMania): design reference only due to restrictive upstream licenses.
- [Web Balatro](https://github.com/W0W53R/web-balatro): fallback browser implementation; license needs verification before copying code.

## Security requirements
- No copyrighted game files in Git or public release artifacts.
- Local-network-only upload endpoint; pair with session PIN, expire sessions and rate-limit attempts.
- Stream incoming files into a staging area and enforce size limits; verify ZIP signature and archive paths.
- Atomically replace game files only after integrity checks. Keep saves separate and protect against accidental deletion.
- Close local server when unused; no remote cloud upload.
- Never require users to expose their TV's SSH port to the Internet.

## Feasibility gates before claiming Balatro support
1. Confirm target TV architecture, disk space and renderer.
2. Build and launch an ordinary LÖVE 11.5 smoke-test game.
3. Test user-owned Balatro game archive, required Lua facilities and shaders.
4. Verify controls, audio, loading, save/reload and 30-minute stability.
5. Package IPK and test install/uninstall/update on a TV before Homebrew submission.
