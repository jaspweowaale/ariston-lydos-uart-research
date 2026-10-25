#!/usr/bin/env python3
"""Generate a fully synthetic, fabricated UART test dataset for the offline
test harness -- ships IN this repository, so `run_tests.sh` works on a fresh
clone with no external/private data of any kind.

Every byte sequence in this file is INVENTED for testing purposes. None of it
is derived from, copied from, or numerically matches any real captured
device traffic (not the excluded third-party Velis 80 captures this project
used during private development, and not the Velis Evo reference project's
own published example frames either -- those are different real sources,
and this generator intentionally avoids reproducing bytes from either).
The only thing "real" here is the protocol RULE SET (framing, checksum,
register addresses) that PROTOCOL.md documents and attributes to its
sources -- applying a documented rule to invented numbers is not the same
as reproducing someone else's captured data, and this file does the former
only.

Output format matches what firmware/core's test harness already expects:
one line per frame, "<timestamp_ms> <hex bytes of the whole frame>",
written to two per-direction files (request = gateway->MCU, response =
MCU->gateway), per named scenario, under streams_synthetic/.
"""
import struct
from pathlib import Path

OUT_DIR = Path(__file__).resolve().parent / "streams_synthetic"

REQUEST_ID = 0x41   # existing factory gateway -> main MCU
RESPONSE_ID = 0x14  # main MCU -> existing factory gateway


def checksum(frame_bytes_without_chk):
    return sum(frame_bytes_without_chk) & 0xFF


def frame(id_byte, cmd, payload):
    body = [0xC3, id_byte, cmd, len(payload)] + list(payload)
    return body + [checksum(body)]


def le16(addr_or_value):
    return [addr_or_value & 0xFF, (addr_or_value >> 8) & 0xFF]


def temp_to_raw_le(celsius):
    return le16(round(celsius * 10))


def fmt_line(ts_ms, frame_bytes):
    return f"{ts_ms:.3f} " + " ".join(f"{b:02X}" for b in frame_bytes) + "\n"


def write_scenario(name, req_lines, resp_lines):
    OUT_DIR.mkdir(exist_ok=True)
    (OUT_DIR / f"{name}_request.txt").write_text("".join(req_lines))
    (OUT_DIR / f"{name}_response.txt").write_text("".join(resp_lines))
    print(f"  {name}: {len(req_lines)} request frames, {len(resp_lines)} response frames")


def scenario_boot():
    """Fabricated cold-boot-style exchange: an unrecognised handshake command
    (deliberately filled with obviously-placeholder 0xAA bytes, not any real
    handshake payload from any source) followed by two ordinary register
    reads. cmd=0x52 is not decoded/correlated by this project's own logic
    either way (see ariston_passive_core.cpp handle_request_frame) -- it is
    included here only so the harness also exercises an "unrecognised
    command, nothing to correlate" frame inside a realistic sequence."""
    req, resp = [], []
    req.append(fmt_line(0, frame(REQUEST_ID, 0x52, [0xAA, 0xAA, 0xAA])))
    resp.append(fmt_line(20, frame(RESPONSE_ID, 0x52, [0xAA, 0xAA, 0xAA, 0xAA])))

    req.append(fmt_line(100, frame(REQUEST_ID, 0x25, le16(0x2105))))  # power
    resp.append(fmt_line(118, frame(RESPONSE_ID, 0x25, [0x01, 0x00, 0x00, 0x00])))

    req.append(fmt_line(200, frame(REQUEST_ID, 0x25, le16(0x2D79))))  # set temperature
    resp.append(fmt_line(222, frame(RESPONSE_ID, 0x25, temp_to_raw_le(50.0) + [0x00, 0x00])))
    return req, resp


def scenario_poll_cycle():
    """Fabricated steady-state poll cycle, repeated 3 times with IDENTICAL
    values each cycle -- exercises value-level dedup across realistic
    traffic density (not just the two-far-apart-reads case already covered
    elsewhere), a batched 0x23 read, and confirms an UNDECODED register
    (0xD84B) is parsed/correlated but never published as a decoded value."""
    req, resp = [], []
    enum_regs = [
        (0x2105, [0x01, 0x00, 0x00, 0x00]),
        (0x2106, [0x00, 0x00, 0x00, 0x00]),
        (0x210A, [0x00, 0x00, 0x00, 0x00]),
        (0x27DD, [0x02, 0x00, 0x00, 0x00]),
    ]
    tank_addrs = [0x1060, 0x1368, 0x1369, 0x136A, 0x136B]
    tank_temps_c = [26.0, 26.1, 26.2, 25.9, 26.0]

    cycle_period_ms = 1200
    for cycle in range(3):
        base = cycle * cycle_period_ms
        t = base
        for addr, raw_bytes in enum_regs:
            req.append(fmt_line(t, frame(REQUEST_ID, 0x25, le16(addr))))
            resp.append(fmt_line(t + 18, frame(RESPONSE_ID, 0x25, raw_bytes)))
            t += 40

        req.append(fmt_line(t, frame(REQUEST_ID, 0x25, le16(0x9E6E))))  # displayed temp
        resp.append(fmt_line(t + 18, frame(RESPONSE_ID, 0x25, temp_to_raw_le(25.0) + [0x00, 0x00])))
        t += 40

        req.append(fmt_line(t, frame(REQUEST_ID, 0x25, le16(0xD84B))))  # UNDECODED register
        resp.append(fmt_line(t + 18, frame(RESPONSE_ID, 0x25, [0x00, 0x00, 0x00, 0x00])))
        t += 40

        batch_payload = []
        for a in tank_addrs:
            batch_payload += le16(a)
        req.append(fmt_line(t, frame(REQUEST_ID, 0x23, batch_payload)))
        value_payload = []
        for c in tank_temps_c:
            value_payload += temp_to_raw_le(c)
        resp.append(fmt_line(t + 30, frame(RESPONSE_ID, 0x23, value_payload)))

    return req, resp


def scenario_setpoint_transition():
    """Fabricated setpoint-change sequence: 50.0C -> 55.0C -> 48.0C, each
    held for several consecutive reads before stepping -- deliberately
    different numbers from this project's own real (excluded) controlled
    experiment (which used 72/75/70) and from the Evo reference project's
    published example (69.0C), so this cannot be mistaken for either."""
    req, resp = [], []
    plan = [50.0] * 4 + [55.0] * 3 + [48.0] * 2
    t = 0
    for c in plan:
        req.append(fmt_line(t, frame(REQUEST_ID, 0x25, le16(0x2D79))))
        resp.append(fmt_line(t + 18, frame(RESPONSE_ID, 0x25, temp_to_raw_le(c) + [0x00, 0x00])))
        t += 220
    return req, resp


def scenario_corrupted_stream():
    """Response-direction-only scenario: a continuous stream mixing valid
    frames, a frame with a deliberately corrupted checksum, raw garbage
    bytes between frames, and a truncated/interrupted frame followed by a
    long gap -- tests frame synchronization and checksum validation inside
    one continuous realistic stream (complementing, not duplicating, the
    isolated unit tests already in test_harness.cpp's Part 2)."""
    resp = []

    good1 = frame(RESPONSE_ID, 0x25, temp_to_raw_le(51.0) + [0x00, 0x00])
    resp.append(fmt_line(0, good1))

    bad = frame(RESPONSE_ID, 0x25, temp_to_raw_le(52.0) + [0x00, 0x00])
    bad[-1] ^= 0xFF  # corrupt the checksum byte specifically
    resp.append(fmt_line(50, bad))

    garbage = [0x11, 0x22, 0x33, 0x44, 0x55]
    resp.append(fmt_line(100, garbage))

    good2 = frame(RESPONSE_ID, 0x25, temp_to_raw_le(53.0) + [0x00, 0x00])
    resp.append(fmt_line(150, good2))

    # Truncated: declares a 4-byte payload but this "line" only delivers 2 of
    # them, then the stream goes quiet well past FRAME_TIMEOUT_MS (500ms).
    truncated = [0xC3, RESPONSE_ID, 0x25, 0x04, 0x00, 0x00]
    resp.append(fmt_line(200, truncated))

    good3 = frame(RESPONSE_ID, 0x25, temp_to_raw_le(54.0) + [0x00, 0x00])
    resp.append(fmt_line(200 + 600, good3))  # gap > 500ms triggers the watchdog resync

    return [], resp


def main():
    print(f"Writing synthetic scenarios to {OUT_DIR}")
    for name, fn in [
        ("synthetic_boot", scenario_boot),
        ("synthetic_poll_cycle", scenario_poll_cycle),
        ("synthetic_setpoint_transition", scenario_setpoint_transition),
        ("synthetic_corrupted_stream", scenario_corrupted_stream),
    ]:
        req, resp = fn()
        write_scenario(name, req, resp)
    print("Done. None of this data is derived from any real captured device traffic.")


if __name__ == "__main__":
    main()
