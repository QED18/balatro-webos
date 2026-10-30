# Test record

## 0.3.11-beta.1

| Check | Result |
| --- | --- |
| Source policy check | Passed with `python3 scripts/verify-source.py` |
| Native ARM package build | Passed on Linux x86_64 with the webOSbrew Buildroot SDK |
| Package identity | `io.github.qed18.balatro` |
| Package SHA-256 | `0b9cf36e93e9b39456f92eaab24bfe17ee3aa12c22fee273c2e1606da2f8421e` |
| Package notices | Verified in the IPK under `third_party_licenses/` |
| Clean IPK install | Passed on LG webOS 9.2 in Developer Mode |
| First-run portal | Passed; native portal served on the TV's private LAN address |
| Prior owner-file gameplay checks | macOS `.love`, Windows `Balatro.exe`, Magic Remote pointer, and DualSense were exercised on the preceding test package |

## Required beta follow-up

Run one complete owner-file transfer and relaunch using the final `io.github.qed18.balatro` ID before changing this beta release to a stable release. Record the TV model and webOS version when testing additional hardware.
