# Test Results

Reproducible by anyone, from this repository alone — no external or private
data required. Run with:

```sh
cd test
sh run_tests.sh
```

## Last run

Compiled with `g++ -std=c++17 -O2` against `firmware/core/ariston_passive_core.cpp`
and run directly (not estimated). Test data generated fresh by
`generate_synthetic_streams.py` immediately before this run.

```
=== Part 1: replay of fully synthetic, fabricated UART scenarios (no real/private capture data) ===

--- synthetic_boot: handshake (uncorrelated) + 2 ordinary register reads ---
  PASS: synthetic_boot: all 3 request frames (handshake + 2 reads) parse and checksum-validate
  PASS: synthetic_boot: all 3 response frames parse and checksum-validate
  PASS: synthetic_boot: the power-register read decodes to exactly one raw (MATCHED_ENUM) publish
  PASS: synthetic_boot: the set-temperature read decodes to the fabricated 50.0C value

--- synthetic_poll_cycle: 3 identical repeated cycles -> dedup + UNDECODED-register silence ---
  PASS: synthetic_poll_cycle: all 21 request frames (7 per cycle x 3 cycles) parse and checksum-validate
  PASS: synthetic_poll_cycle: all 21 response frames parse and checksum-validate
  PASS: synthetic_poll_cycle: 6 distinct temperature registers (displayed temp + 5 tank addresses) each publish exactly once despite 3 identical repeated cycles
  PASS: synthetic_poll_cycle: 4 distinct raw/enum registers (power/eco/anti-legionella/mode) each publish exactly once, same dedup behavior
  PASS: synthetic_poll_cycle: the UNDECODED register (0xD84B) is parsed/correlated but never published as a decoded value, in any of the 3 cycles

--- synthetic_setpoint_transition: fabricated 50.0C -> 55.0C -> 48.0C ---
  decoded sequence: 50.0 55.0 48.0
  PASS: synthetic_setpoint_transition: decoder reproduces the fabricated 50.0C -> 55.0C -> 48.0C step sequence in order,
        with repeated same-value reads correctly deduped to one publish per step (deliberately different numbers from
        this project's own excluded real experiment and from the Evo reference project's published example, so this
        result cannot be mistaken for either)

--- synthetic_corrupted_stream: resync across corruption, garbage, and an interrupted frame ---
  PASS: synthetic_corrupted_stream: exactly 4 byte sequences complete frame parsing (2 clean + 1 corrupted-checksum + 1
        clean after the gap; the truncated frame and the 5 raw garbage bytes never complete parsing at all, as expected)
  PASS: synthetic_corrupted_stream: 3 of those 4 are checksum-valid; the deliberately corrupted one is correctly flagged invalid
  PASS: synthetic_corrupted_stream: all 3 valid frames correctly report as unmatched (no request was ever fed in this
        response-only scenario, so there is nothing to correlate against -- confirms that path doesn't misfire)

=== Part 2: synthetic malformed / interrupted-frame handling (isolated unit tests) ===
  PASS: two genuinely valid frames both recognised despite a corrupted frame and garbage in between
  PASS: the corrupted-checksum frame is flagged invalid, not silently accepted
  PASS: after an interrupted mid-frame stream (gap > FRAME_TIMEOUT_MS), the next valid frame is correctly resynced and decoded
  PASS: an exact-duplicate frame arriving within DUPLICATE_SUPPRESS_MS is not re-processed a second time
  PASS: raw seen-count still reflects both physical frames for diagnostics, even though only one was acted on
  PASS: an unchanged decoded value queried twice, far apart in time, is published once, not twice (value-level dedupe, not just timing-window dedupe)

=== RESULT: 19 passed, 0 failed ===
```

## What this does and doesn't prove

**Proves:** the decode/correlation logic (frame synchronization, checksum
validation, corrupted/truncated-frame handling, request/response
correlation, register-tier-gated publishing, and value-level dedup) behaves
correctly against protocol-valid traffic, using this project's own
documented framing rules.

**Does not prove:** that this traffic shape is what a real Ariston Lydos
actually produces on its CN6 connector. No real Lydos capture exists yet —
see `../HARDWARE_VALIDATION_CHECKLIST.md` and
`../firmware/docs/INSTALLATION_PREREQUISITES.md` for what's needed before
that question can be answered. This test suite validates the software; it
cannot validate a hardware assumption that hasn't been checked yet.

## Historical note (not reproduced here)

This project's private development additionally ran this same decoder logic
against 2,675 real frames from a third party's own capture of a different
Ariston unit (see `README.md` for why that data isn't included here). That
run also passed 9/9 of its own checks. It is cited here as context for how
this logic was originally developed and validated — not as a result anyone
cloning this repository can reproduce, since the underlying data is
intentionally not included.
