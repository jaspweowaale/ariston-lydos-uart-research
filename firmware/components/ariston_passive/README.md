# `ariston_passive` ESPHome component — status

**Build-verified (2026-10-25).** ESPHome 2026.6.5 was installed in a dedicated,
isolated Python venv on a dedicated development machine, never touching
any production venv/service) and `esphome compile lydos_passive_monitor.yaml`
succeeds end-to-end for the `esp32-s3-devkitc-1` target. Exact commands,
versions, and firmware SHA-256 are in `../../BUILDING.md`.

## What IS actually tested

All decode/correlation/dedupe logic lives in
`../../core/ariston_passive_core.h` / `.cpp` — a plain, framework-agnostic
C++ file with zero ESPHome or Arduino dependency. That file is compiled with
plain `g++` and run against a fully synthetic, fabricated dataset
(`../../test/generate_synthetic_streams.py`) covering frame sync, checksum
validation, corrupted/interrupted frames, request/response correlation, and
register decoding. **19/19 tests pass, with real numbers, reproducible by
anyone from this repository alone** — see `../../test/TEST_RESULTS.md`. (The
logic was originally developed against 2,675 real frames from a third
party's own capture of a related product — 9/9 passed at the time — but
that data isn't included here; see `../../test/README.md`.)

This directory also contains a **packaged copy** of those same two files
(`ariston_passive_core.h`/`.cpp`, byte-identical to the canonical copy at the
time of copying — confirmed via `diff`) because ESPHome's `external_components`
mechanism only pulls in the declared component directory itself, not a sibling
`core/` directory one level up. The packaged copy is what actually gets
compiled into the firmware; the canonical copy in `../../core/` is what the
standalone test harness validates. **Keep both in sync if the decode logic
ever changes** — this is a real, documented duplication, not an oversight.

## What this wrapper does, and does not, add

`ariston_passive.h` / `.cpp` in this directory are a thin adapter:
`AristonPassiveComponent::loop()` reads available bytes off two
`uart::UARTComponent` instances and calls `monitor_.feed_request_byte()` /
`feed_response_byte()` — that's the entire bridge. Every actual decision
(frame sync, checksum, correlation, dedupe, which registers get published)
happens inside the already-tested core. The wrapper adds no parsing logic of
its own. **This has now been verified by an actual build** — the component
compiles and links cleanly against ESPHome 2026.6.5 / ESP-IDF 5.5.4, after one
integration fix (the packaged-copy workaround above; see
`../../BUILDING.md` for the exact error and fix).

`__init__.py` / `sensor.py` / `text_sensor.py` are ESPHome Python codegen
scaffolding — **now confirmed to run correctly through ESPHome's real config
validator and compiler** (`esphome config` and `esphome compile` both
succeed). One schema issue was found and fixed at the top-level YAML (an
invalid `encryption:` key under `ota.esphome` not supported by this ESPHome
version) — not a bug in this component's own schema.

## Hard structural guarantee — now checked two ways

Every UART interaction in this directory is read-only:

```
grep -n "write_array\|write_byte\|->write(" ariston_passive.cpp ariston_passive.h ../../core/ariston_passive_core.*
```

returns nothing (the word "write" does appear in two comments, neither a
function call). There is no code path anywhere in this component, or in the
core it wraps, that transmits a byte onto either uart bus.

**This is now also confirmed at the ESPHome-resolved-config level, not just
by source inspection**: `esphome config lydos_passive_monitor.yaml` was run
and its full resolved output was grepped for `tx_pin` — zero matches on
either uart bus, and both buses resolve to `mode: {input: true, output:
false}`. See `../../BUILDING.md` §6.

## Known gap, the `__init__.py` `_require_rx_only` placeholder

`__init__.py`'s `_require_rx_only()` function is a documented no-op
placeholder (it returns its input unchanged) — the real RX-only enforcement
for this pass was done externally, by inspecting `esphome config`'s resolved
output (see above), not by a config-validation-time check inside this
component's own schema. Implementing a real `cv.Invalid` check that inspects
the referenced uart component's own config dict for `CONF_TX_PIN` is possible
future work, not required for this pass's build-validation goal — flagged
honestly rather than claimed as done.

## Before relying on this wrapper on real hardware

Compiling is not the same as working against real Lydos bytes. Still
required, unchanged: the de-energized visual inspection
(`../../HARDWARE_VALIDATION_CHECKLIST.md`), electrical-isolation verification
(`../../SAFETY.md` §2), and a real capture to confirm
baud/parity/pin mapping for the Lydos specifically. See
`../../docs/INSTALLATION_PREREQUISITES.md`.
