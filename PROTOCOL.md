# Protocol — What's Confirmed, Matched, and Unverified

This document uses three confidence tiers throughout. Treat the tier as part
of the fact, not a footnote:

- **CONFIRMED** — independently verified against real captured data via a
  controlled experiment (a known action taken at a known time, with the
  decoded value changing to match, more than once).
- **MATCHED (cross-model)** — the same address/behavior is independently
  documented or observed on two different real Ariston products, built and
  published by two unrelated parties. Strong evidence of a shared protocol
  family, but not itself a confirmation for any *specific third* product
  (including the Lydos this project targets).
- **CLAIMED / UNVERIFIED** — stated by exactly one source (typically a forum
  comment), never independently reproduced or byte-verified.

## 1. What this project actually targets

The goal is local, read-only telemetry from an **Ariston VELIS Lydos**
electric water heater. **No real capture of a Lydos's own UART traffic has
ever been obtained, by this project or (as far as its research found)
anyone publicly.** Everything below about the Lydos specifically is
inference from related products, clearly labeled as such.

## 2. Reference products actually captured/documented

| Product | Status | Source |
|---|---|---|
| **Ariston Velis 80 Wi-Fi** | CONFIRMED, byte-exact — 2,675 real captured UART frames, independently re-verified checksum-valid | Decoded from a real logic-analyzer capture, originally made by a third-party researcher and separately analyzed in this project's research |
| **Ariston Velis Evo Plus 100 EU** | CONFIRMED, working on real hardware (read + write), independently published | A separate, independent open-source project — see §6, Credits |
| **Ariston Lydos** (this project's actual target) | **No real capture exists.** Only a CN6 connector description and UART parameters from one unverified forum comment. | GitHub issue discussion (see §6) |

## 3. Framing and checksum — MATCHED across two real products

```
ESP/gateway -> heater:  C3 <ID> <CMD> <LEN> <payload...> <checksum>
heater -> ESP/gateway:  C3 <ID> <CMD> <LEN> <payload...> <checksum>
checksum = sum of all preceding bytes, mod 256
```

This exact framing and checksum, byte-for-byte, is independently confirmed
on both the Velis 80 Wi-Fi (via real capture) and the Velis Evo Plus 100 EU
(via an independent open-source implementation) — two different products,
two unrelated research efforts, same result. One complete request/response
exchange (a `0x52` initialization handshake) was found byte-identical,
checksum included, across both.

**UART parameters, MATCHED:** 9600 baud, 8 data bits, no parity, 1 stop bit
("8N1"), 5V logic level — confirmed independently on both reference
products.

**For the Lydos specifically:** one forum comment claims **19200 baud, even
parity** for an internal link on a Lydos-family board — never byte-verified,
and not even confirmed to be describing the same connector referenced in §5.
This project's firmware tries 9600/8N1 first (now backed by two real
products) and documents 19200/even as a fallback only — see
[firmware/lydos_passive_monitor.yaml](firmware/lydos_passive_monitor.yaml).

## 4. Register map

16 exact register addresses were found to match between the two reference
products' independently-documented poll tables, including the complete
`0x52` handshake above. Key addresses:

| Address | Field | Tier |
|---|---|---|
| `0x2D79` | Set (target) temperature, °C × 10 | **CONFIRMED** (controlled experiment: 72.0°C → 75.0°C → 70.0°C observed, in order, on real data) |
| `0x9E71` | Same field, alternate/diagnostic address | **CONFIRMED**, same method |
| `0x9E6E` | Panel-displayed temperature | MATCHED |
| `0x1060`, `0x1368`–`0x136B` | Tank thermistor cluster | MATCHED |
| `0x2105` / `0x2106` / `0x210A` | Power / ECO / anti-legionella registers | MATCHED — **raw bytes only, no confirmed boolean semantics.** Both reference products poll these addresses, but this project's own analysis explicitly did not locate reliable on/off byte semantics in the real captured data. Exposed as raw hex, never as a fabricated true/false. |
| `0x27DD` | Mode register | MATCHED — same caveat, **plus**: whether a plain Lydos even supports the Evo's documented MAN/P1/P2/P1+P2 schedule modes is unestablished. |

Full detail, including every address checked and the ones present in only
one reference product's table, is reproducible from `firmware/core/` and
the test results in `test/` — this document summarizes, it doesn't hide
anything behind "trust us."

**Known hazard, confirmed on the Evo reference product:** reading register
`0x0000` (within the `0x0000`–`0x0005` range) reset that unit's main MCU and
lost its clock setting. Any register scan should start at `0x0010` or above.
This project's own firmware never issues read/scan commands at all (it's
receive-only — see [BUILDING.md](BUILDING.md)'s "Verify the receive-only
guarantee yourself" section for how that's checked against the actual
compiled build, not just the source YAML) but this is important context if
you ever experiment with an active/write-capable tool against this protocol
family.

## 5. The CN6 connector — unresolved discrepancy, read before assuming anything

The Velis Evo reference implementation documents its `CN6` as a 6-pin
connector: pins 1–4 are the active UART link (TX/RX/+5V/GND), pins 5–6 are
unused SWD debug pins.

A single, never-independently-verified forum comment about a Lydos-family
board describes its own `CN6` pin 6 as a **24V-referenced presence signal**
— a completely different function from "unused debug pin." Either the two
products' `CN6` connectors are genuinely different layouts despite sharing a
name, or one of the two secondhand descriptions is simply wrong.

**This has never been resolved with a photograph of a real Lydos board.**
Do not treat the Velis Evo's pinout as a wiring instruction for a Lydos. See
[HARDWARE_VALIDATION_CHECKLIST.md](HARDWARE_VALIDATION_CHECKLIST.md).

## 6. Credits and provenance

- The `C3`-framed binary protocol, as implemented on the Velis 80 Wi-Fi, was
  originally reverse-engineered by contributors to a public Elektroda.com
  forum thread ("Interior and reverse engineering of the Ariston Velis 80
  Wi-Fi electric water heater on ESP32") — see that site's `topic4148462`.
- The Velis Evo Plus 100 EU implementation this project's register-map
  comparison relies on is an independent, separately-published open-source
  project (credited in its own README as building on the same Elektroda
  research). This project does not vendor that implementation's code — only
  its publicly-documented, factual register addresses are reproduced here,
  with attribution. Check that project's own repository directly if you want
  its original source.
- The Lydos's CN6 connector description and claimed UART parameters
  originate from unverified comments on a public GitHub issue thread for an
  existing Ariston Home Assistant cloud integration.
- One specific example frame used in this project's own test suite (a
  duplicate-detection check, address `0x2D79`, value 69.0°C) —
  `C3 41 33 04 79 2D B2 02 95` — is forum user **DeDaMrAz**'s own
  already-public, self-published illustrative example from the Elektroda
  thread above, reused here with attribution because the original poster
  published this exact byte sequence themselves as a public example, which
  this project treats as a different provenance tier from the same
  contributor's bulk private capture session (not included — see
  [test/README.md](test/README.md)).

No account, login, or access-restricted material was used to obtain any of
the above — everything cited here is public, attributed, and independently
re-verified where this project claims "CONFIRMED."
