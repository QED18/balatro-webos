#!/usr/bin/env python3
"""Fast, dependency-free checks that never unpack or accept game files."""
from __future__ import annotations

import json
import pathlib
import sys
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
errors: list[str] = []

def require(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)

for name in ("icon.png", "largeIcon.png"):
    image = ROOT / "app" / name
    require(image.read_bytes().startswith(b"\x89PNG\r\n\x1a\n"), f"{name} is not a PNG")

metadata = json.loads((ROOT / "native/webos/appinfo.json.in").read_text())
require(metadata["type"] == "native", "smoke test must be a native webOS app")
require(metadata["main"] == "bin/balatro-webos-smoke", "native entry point changed")
require(metadata["icon"] == "icon.png", "smoke package icon missing")
source = (ROOT / "native/sample/main.lua").read_text()
for required in ("love.filesystem.write", "love.filesystem.read", "love.audio.newSource", "love.keypressed"):
    require(required in source, f"smoke sample does not exercise {required}")
for forbidden in ("Balatro.exe", "Balatro.love", "http://", "https://", "socket", "upload"):
    require(forbidden not in source, f"forbidden sample content: {forbidden}")

archive = ROOT / ".verify-smoke.love"
try:
    with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as out:
        out.write(ROOT / "native/sample/main.lua", "main.lua")
    with zipfile.ZipFile(archive) as packed:
        require(packed.namelist() == ["main.lua"], "smoke archive must contain only main.lua")
finally:
    archive.unlink(missing_ok=True)

if errors:
    print("FAIL")
    print("\n".join(f"- {error}" for error in errors))
    sys.exit(1)
print("PASS: source, icon metadata, and no-assets smoke archive verified")
