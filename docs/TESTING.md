# Test record

## 0.3.11-beta.1

| Check | Result |
| --- | --- |
| Source policy check | Passed with `python3 scripts/verify-source.py` |
| Native ARM package build | Passed on Linux x86_64 with the webOSbrew Buildroot SDK |
| Package identity | `io.github.qed18.balatro` |
| Release asset SHA-256 | `0b9cf36e93e9b39456f92eaab24bfe17ee3aa12c22fee273c2e1606da2f8421e` |
| Package notices | Verified in the IPK under `third_party_licenses/` |
| Clean IPK install | Passed on LG webOS 9.2 in Developer Mode |
| First-run portal | Passed; native portal served on the TV's private LAN address |
| Final macOS owner-file import | Passed: `Balatro.love` transferred, validated, stored, and the new `io.github.qed18.balatro` app relaunched into its game host |
| Prior owner-file gameplay checks | Windows `Balatro.exe`, Magic Remote pointer, and DualSense were exercised on the preceding test package |

## Beta follow-up

Record the TV model and webOS version when testing additional hardware. Test a Windows/Proton import and controller session on the public app ID before promoting this beta to a stable release.
