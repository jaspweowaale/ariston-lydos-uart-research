# ariston-lydos-uart-research

**Status: UNTESTED ON REAL HARDWARE. Not affiliated with or endorsed by Ariston.**

This is a research project toward local, cloud-free, read-only monitoring of
an Ariston VELIS Lydos electric water heater (water temperature, target
temperature, heating status, operating mode) over its internal `CN6` UART
connector — no cloud account, no Wi-Fi gateway dependency, nothing written
back to the heater.

It is **not** a finished product. Nobody has yet connected this to a real
Lydos. What exists today is:

- A protocol decoder/correlator. It was originally developed and validated
  against **2,675 real captured UART frames** from a related Ariston
  product (see [PROTOCOL.md](PROTOCOL.md) for exactly which product, and why
  this is a related-but-different model from the actual Lydos) — that real
  capture data is not included here (see below), so the bundled, publicly
  reproducible test suite instead exercises the same decode logic against a
  fully synthetic, fabricated dataset: **19/19 checks pass**, reproducible
  by anyone from this repository alone (see [test/TEST_RESULTS.md](test/TEST_RESULTS.md)).
- A passive, **receive-only** ESPHome firmware design around that decoder —
  structurally incapable of transmitting anything onto the UART bus (see
  [PROTOCOL.md](PROTOCOL.md) §4 for how "receive-only" is verified, not just
  claimed).
- A real, reproducible build: compiles cleanly against a pinned ESPHome
  version (see [BUILDING.md](BUILDING.md)).
- A documented, honest list of exactly what is still unknown and what has to
  happen, in order, before this touches a real appliance (see
  [HARDWARE_VALIDATION_CHECKLIST.md](HARDWARE_VALIDATION_CHECKLIST.md) and
  [SAFETY.md](SAFETY.md)).

## Why this might not work on your Lydos

The decoder is validated against real data from a **different Ariston
product line** (see [PROTOCOL.md](PROTOCOL.md)). There is real cross-model
evidence the underlying protocol family is shared, but:

- The Lydos's own UART baud rate/parity has only ever been described in an
  unverified forum comment, never independently measured.
- The exact `CN6` pin layout and function on a plain (non-Hybrid) Lydos has
  never been photographed or confirmed by anyone, publicly, as far as this
  project's research could find.
- The exact register map could differ from both reference products used
  here.

**This project needs real Lydos owners to help close those gaps.** See
[HARDWARE_VALIDATION_CHECKLIST.md](HARDWARE_VALIDATION_CHECKLIST.md) — if you
own a Lydos and are comfortable doing a de-energized visual inspection, your
photos and findings are the single most useful contribution possible right
now.

## Project layout

```
firmware/
  lydos_passive_monitor.yaml     ESPHome config — two independent, receive-only
                                 UART buses, zero transmit capability
  core/                          Framework-agnostic C++ decode/correlation logic
                                 (frame sync, checksum validation, request/response
                                 correlation, dedup) — this is what's actually tested
  components/ariston_passive/    ESPHome external component wrapping core/
  docs/                          Detailed hardware BOM, HA entity mapping,
                                 installation prerequisites, outstanding questions
test/
  test_harness.cpp               Standalone test harness for core/ (plain g++, no
                                 ESPHome needed)
  generate_synthetic_streams.py  Builds the fully synthetic, fabricated test
                                 dataset this repo's test suite runs against by
                                 default — no external data needed
  prepare_test_streams.py        OPTIONAL: regenerates per-direction byte streams
                                 from your OWN decoded capture CSVs, if you have one
  run_tests.sh                   Generates the synthetic dataset and runs the
                                 harness against it
PROTOCOL.md                      What's confirmed, what's inferred, what's unverified
SAFETY.md                        Mains-appliance electrical safety — read before any
                                  physical work
BUILDING.md                      Exact install/compile commands, pinned versions
HARDWARE_VALIDATION_CHECKLIST.md How to help, without connecting anything live
CONTRIBUTING.md
```

## What's deliberately *not* included in this repository

- **No raw capture data.** The 2,675 frames used to originally develop and
  validate the decoder were captured by a third party on their own
  (different) Ariston unit, obtained during this project's research for
  analysis purposes. We don't have a clear redistribution right over
  someone else's private capture session, so those raw byte streams are not
  shipped here — only a historical summary of that result is reported (see
  [test/README.md](test/README.md)). The test suite you can actually run
  from this repository uses a separate, fully synthetic/fabricated dataset
  generated by `test/generate_synthetic_streams.py` — nothing in it is
  derived from real captured device traffic. The tooling to run your own
  real capture through the same decoder is also provided (`prepare_test_streams.py`),
  if you have one. See [PROTOCOL.md](PROTOCOL.md) and
  [test/](test/) for details.
- **No vendored code from other projects.** This project builds on protocol
  groundwork independently published by other researchers (credited
  throughout [PROTOCOL.md](PROTOCOL.md)) but does not copy their source
  code — only cites publicly-documented facts (e.g. register addresses, which
  are protocol data, not copyrightable expression) with attribution and a
  link to the original.

## License

See [LICENSE](LICENSE) (proposed: MIT, for the original code in this
repository only — see that file and [PROTOCOL.md](PROTOCOL.md) for why
third-party-sourced facts are cited rather than relicensed).

## Disclaimer

This project is independent, unofficial research. It is not affiliated
with, endorsed by, or supported by Ariston Thermo S.p.A. Interacting with
the internals of a mains-powered appliance carries real electrical risk —
read [SAFETY.md](SAFETY.md) before doing anything physical. Nothing in this
repository has been tested against a real Ariston Lydos.
