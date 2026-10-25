#!/usr/bin/env python3
"""OPTIONAL utility: reformat your OWN decoded UART capture CSVs into the
per-direction stream format this project's test harness understands.

This is not needed to run the bundled test suite -- that uses
generate_synthetic_streams.py's fully fabricated data by default (see
run_tests.sh / test/README.md). Use this script only if you have your own
real CN6 capture, already decoded into CSV rows of
(start_sample, dir, id_hex, cmd_hex, len, payload_hex, chk_hex), and want to
run it through the same PassiveMonitor decoder for your own testing.

This project does not ship any real captured device traffic -- the private
development data used to validate ariston_passive_core.cpp was a third
party's own capture of a different Ariston unit, and is excluded from this
repository because we don't hold a redistribution right over it (see
test/README.md). Your own capture of your own device is yours to use here.

Usage:
    python3 prepare_test_streams.py <path-to-your-csv-directory>

Output goes to streams_from_own_capture/ (deliberately a different
directory from streams_synthetic/, so your real data is never confused with
or silently overwritten by the bundled synthetic fixtures). test_harness.cpp
does not automatically replay files from this directory -- its Part 1
scenario list is hardcoded to the synthetic stems. To test your own capture,
add a small replay_scenario("streams_from_own_capture", "<your-stem>", mon)
call of your own, following the pattern already used for the synthetic
scenarios.
"""
import csv
import sys
from pathlib import Path

SAMPLE_RATE_HZ = 2_000_000
OUT_DIR = Path(__file__).resolve().parent / "streams_from_own_capture"


def sample_to_ms(sample: int) -> float:
    return sample / SAMPLE_RATE_HZ * 1000.0


def row_to_frame_bytes(row) -> list[int]:
    idb = int(row["id_hex"], 16)
    cmd = int(row["cmd_hex"], 16)
    ln = int(row["len"])
    payload = [int(x, 16) for x in row["payload_hex"].split()] if row["payload_hex"].strip() else []
    chk = int(row["chk_hex"], 16)
    assert len(payload) == ln, f"len mismatch: declared {ln}, got {len(payload)} bytes"
    return [0xC3, idb, cmd, ln] + payload + [chk]


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        print("ERROR: you must pass the path to your own CSV directory.")
        return 1
    in_dir = Path(sys.argv[1])
    if not in_dir.is_dir():
        print(f"ERROR: {in_dir} is not a directory.")
        return 1

    OUT_DIR.mkdir(exist_ok=True)
    csv_files = sorted(in_dir.glob("*.csv"))
    print(f"Found {len(csv_files)} capture CSVs in {in_dir}")

    for csv_path in csv_files:
        stem = csv_path.stem
        req_lines = []  # TX = gateway->MCU = "request" stream
        resp_lines = []  # RX = MCU->gateway = "response" stream
        n_rows = 0
        with open(csv_path, newline="") as fh:
            for row in csv.DictReader(fh):
                n_rows += 1
                ts_ms = sample_to_ms(int(row["start_sample"]))
                frame_bytes = row_to_frame_bytes(row)
                hexstr = " ".join(f"{b:02X}" for b in frame_bytes)
                line = f"{ts_ms:.6f} {hexstr}\n"
                if row["dir"] == "TX":
                    req_lines.append(line)
                elif row["dir"] == "RX":
                    resp_lines.append(line)
                else:
                    raise ValueError(f"unexpected dir {row['dir']!r} in {csv_path}")

        req_path = OUT_DIR / f"{stem}_request.txt"
        resp_path = OUT_DIR / f"{stem}_response.txt"
        req_path.write_text("".join(req_lines))
        resp_path.write_text("".join(resp_lines))
        print(f"  {csv_path.name}: {n_rows} rows -> {len(req_lines)} request frames, {len(resp_lines)} response frames")

    print(f"\nWrote per-direction streams to {OUT_DIR}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
