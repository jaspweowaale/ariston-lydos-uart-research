# Offline Tests

`test_harness.cpp` tests `firmware/core/ariston_passive_core.cpp` — the
decode/correlation logic — standalone, with a plain C++17 compiler. No
ESPHome installation needed for this part.

## Running it (fully self-contained, no external data needed)

```sh
sh run_tests.sh
```

This generates a fully synthetic, fabricated test dataset
(`generate_synthetic_streams.py` → `streams_synthetic/`) and replays it
through the real decoder logic. **Last run: 19/19 checks passed.** Exact
output and the specific assertions are in `TEST_RESULTS.md`; the assertions
themselves (in `test_harness.cpp`) are the real specification.

## About the test data — why this project doesn't ship real capture bytes

This project's own private development additionally validated the same
decoder logic against 2,675 real UART frames. That data was originally
captured by a third party, on their own (different) Ariston unit, and
obtained during this project's research for protocol analysis — not
captured by this project's author. Because we don't have a clear
redistribution right over someone else's private capture, **those raw byte
streams are not bundled in this repository.**

For historical context only (not reproduced or re-asserted by the bundled
test suite): that private run independently re-synced and checksum-validated
all 2,675 frames from scratch, reproduced a real controlled-experiment
72.0°C→75.0°C→70.0°C setpoint sequence exactly, correctly rejected
corrupted-checksum frames, recovered cleanly from a simulated interrupted
stream, and deduped both repeated frames and repeated values — 9/9 checks
passed at the time. The **current, public, fully-reproducible** test suite
(19/19, synthetic data only) exercises the same code paths — frame sync,
checksum validation, corruption/interruption recovery, request/response
correlation, register decoding, and a (fabricated) setpoint transition — so
anyone cloning this repository can verify the same properties themselves,
without needing anyone else's private data.

## Running it against your own real capture (optional)

If you have your own real CN6 capture, decoded into CSV rows of
`start_sample, dir, id_hex, cmd_hex, len, payload_hex, chk_hex` (one row per
decoded frame; `dir` is `TX` or `RX`), `prepare_test_streams.py` will
reformat it into the same per-direction stream format used above:

```sh
python3 prepare_test_streams.py /path/to/your/csv/directory
```

This writes to `streams_from_own_capture/` (kept separate from the bundled
`streams_synthetic/`, so your real data is never confused with or
overwritten by the synthetic fixtures). `test_harness.cpp`'s scenario list
is hardcoded to the synthetic stems — to replay your own data through the
decoder, add a short `replay_scenario("streams_from_own_capture",
"<your-stem>", mon)` call of your own, following the pattern already used
for the synthetic scenarios, then attach whatever checks are meaningful for
your data.
