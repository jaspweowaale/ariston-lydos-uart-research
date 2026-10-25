# Hardware Validation Checklist — How You Can Help

**You do not need to connect anything live to contribute.** The single most
valuable thing this project needs right now is information from real Lydos
owners — most of it obtainable with the appliance safely de-energized.

Please read [SAFETY.md](SAFETY.md) in full before doing any of this.

## Step 1 — De-energized visual inspection (the highest-value contribution)

1. Unplug the appliance from mains. Wait several minutes for internal
   capacitors to discharge.
2. Open the casing. **Do not connect any probe, meter, or power at this
   step — this is photography only.**
3. Photograph:
   - The appliance's own rating nameplate (model/SKU, serial number,
     capacity).
   - The main control board's part number / revision silkscreen.
   - The `CN6` connector itself — confirm it exists, note the pin count,
     connector type, and any polarity marking.
   - Any Wi-Fi module/daughterboard, if physically separate from the main
     board, and its own markings.
   - The main MCU chip's markings.
   - The power-supply IC's markings, and any visible transformer or
     optocoupler package nearby (this is the single most useful photo for
     the electrical-isolation question — a real part number lets anyone
     check its actual datasheet topology instead of inferring).
   - Any RJ45 service port, if present.
4. Open an issue using the **Hardware Validation Report** template with
   your photos and findings — even a partial report (e.g. "confirmed CN6 is
   6-pin, couldn't get a clear shot of the main MCU") is useful.

## Step 2 — Resolve the CN6 pin-layout question

This project currently cannot say with confidence what each `CN6` pin does
on a real Lydos — see [PROTOCOL.md](PROTOCOL.md) §5 for the exact
discrepancy between two secondhand descriptions. Your Step 1 photo, plus
(if you're comfortable and qualified) a continuity check with the appliance
**unplugged and discharged** between CN6 pins and known reference points
(the main MCU's own pins, GND, etc.), would directly resolve this.

## Step 3 — Electrical isolation verification (requires an isolation-rated meter)

**Do not skip this even if Steps 1–2 look promising.** With the appliance
unplugged and discharged, use an isolation-rated multimeter to measure
continuity/isolation between `CN6`'s ground reference and the appliance's
mains earth/neutral/line connections. See [SAFETY.md](SAFETY.md) §2 for why
this matters and how to approach it. Report your measurement methodology
and result, not just a conclusion — "measured X ohms between A and B with a
CAT III meter" is far more useful to other contributors than "seems safe."

## Step 4 — Only after Steps 1–3: a real capture

If you're experienced with logic analyzers/UART sniffing and comfortable
with Steps 1–3's findings, a short, **passive, read-only** capture of
`CN6`'s two data lines (not a live connection of this project's firmware —
just a logic analyzer tap) would let anyone test whether 9600/8N1 framing
(see [PROTOCOL.md](PROTOCOL.md) §3) actually appears. This is the step that
would most directly move this project forward, and the one with the most
real risk — please don't attempt it without having genuinely satisfied
Steps 1–3 first, including a USB isolator between your analyzer and your
own computer.

## Reporting back

Please use the issue templates in `.github/ISSUE_TEMPLATE/` — the Hardware
Validation Report template is specifically structured around the steps
above. Partial information is still valuable; please don't feel you need to
complete every step before reporting back.
