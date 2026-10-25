# Building

This firmware has been successfully compiled end-to-end against a real,
pinned ESPHome toolchain, in an isolated environment. It has **not** been
flashed to any physical board (none was available during development) and
has **not** been tested against real Lydos hardware.

## Pinned versions (confirmed working)

| Component | Version |
|---|---|
| ESPHome | 2026.6.5 |
| PlatformIO Core | 6.1.19 |
| platform-espressif32 (pioarduino fork) | release 55.03.39 |
| framework-espidf (ESP-IDF) | 3.50504.0 (ESP-IDF 5.5.4) |
| toolchain-xtensa-esp-elf | 14.2.0+20260121 |
| Target board | `esp32-s3-devkitc-1` |

## Install in an isolated environment

Don't install ESPHome system-wide or into an existing Python environment —
use a dedicated virtual environment:

```sh
python3 -m venv .venv
source .venv/bin/activate   # or .venv\Scripts\activate on Windows
pip install esphome==2026.6.5
```

## Validate the config

```sh
cd firmware
esphome config lydos_passive_monitor.yaml
```

You'll need a `secrets.yaml` alongside the config with placeholder
`wifi_ssid`, `wifi_password`, and `api_key` entries — see ESPHome's own docs
for the `!secret` mechanism. No real secrets are included in this
repository.

## Compile

```sh
esphome compile lydos_passive_monitor.yaml
```

A successful build produces output similar to:

```
RAM:   [=         ]  14.3% (used 46972 bytes from 327680 bytes)
Flash: [====      ]  42.5% (used 779619 bytes from 1835008 bytes)
...
========================= [SUCCESS] Took 57.88 seconds =========================
INFO Successfully compiled program.
```

**Note on reproducible builds:** ESP-IDF embeds the absolute build directory
path and a build timestamp into the compiled binary, so re-running this
build from a different directory path will produce a different firmware
SHA-256 even with identical source — this is expected and does not indicate
a problem. "Compiles successfully" is the signal to check, not binary hash
equality across different build locations.

## Known integration detail

The ESPHome external-component mechanism (`external_components: - source:
{type: local, path: components}`) only pulls the declared `components/`
directory into the build tree — it does not follow a relative include to a
sibling directory one level up. For this reason, `firmware/core/`'s two
files are also packaged as an identical copy inside
`firmware/components/ariston_passive/`. **If you ever modify the decode
logic, keep both copies in sync** — see that directory's own `README.md`.

If you see a build-cache error after adding or changing a source file
inside `components/`, run `esphome clean lydos_passive_monitor.yaml` before
recompiling — ESPHome/CMake's source-file glob is cached at configure time
and won't pick up a newly-added file otherwise.

## Verify the receive-only guarantee yourself

Don't just trust this README — check the actually-resolved build config:

```sh
esphome config lydos_passive_monitor.yaml > resolved_config.txt
grep -n "tx_pin" resolved_config.txt       # expect: zero matches
grep -in "interval\|on_boot\|uart.write\|write_array" resolved_config.txt
# expect: only WiFi's own standard enable_on_boot setting, nothing UART-related
```

## Run the offline tests

No ESPHome installation needed for this part — just Python 3 and a C++17
compiler. Fully self-contained; no external or private data needed:

```sh
cd test
sh run_tests.sh
```

This generates a synthetic, fabricated test dataset and compiles/runs the
harness against it (19/19 checks, as of this writing — see
`test/TEST_RESULTS.md`). If you have your own real Lydos capture data
instead, see `test/README.md` for `prepare_test_streams.py`.
