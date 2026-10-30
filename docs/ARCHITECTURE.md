# Architecture and validation gates

## User journey (target)
Install Webmon from Homebrew Channel → verify the TV's LÖVE runtime using this repository's original smoke test → only then investigate a separate, opt-in compatibility path for a user's own lawfully obtained game files.

## Implementation status
- **Implemented:** setup screen, keyboard/remote event diagnostics.
- **Validated on TV through Webmon:** this repository's original LÖVE smoke test launched, rendered, accepted remote input, and wrote a save counter. Webmon supplied the local-PIN upload portal; no game files were involved.
- **Implemented but unverified on hardware:** a custom native LÖVE 11.5 smoke-test source package that uses a pinned Webmon source checkout. It is experimental and not the supported route.
- **Not yet implemented:** a successful custom-IPK cross-build and validation, Balatro compatibility, gameplay controls, release publication, and Homebrew submission.

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
1. Keep Webmon-based smoke testing repeatable and game-file-free.
2. Independently validate audio, controls, save/reload and 30-minute stability on target TVs.
3. Test only a user-owned Balatro game archive, required Lua facilities and shaders.
4. Build and validate a custom IPK only if Webmon cannot meet the final product needs.
5. Test install/uninstall/update on a TV before any Homebrew submission.

## Smoke-test evidence still required

The source package is deliberately not evidence of runtime compatibility. Before the next milestone, record the TV model/webOS version, Webmon-derived dependency revision, SDK version, IPK SHA-256, `appinfo.json` contents, install/launch outcome, at least several minutes of rendering, D-pad/OK/Back log output, audio result, and a save counter before and after relaunch. If audio fails, record the device-specific SDL/OpenAL error rather than treating it as success.
