# Installation Prerequisites — What Must Happen Before This Is Ever Connected

In order. **None of these have been done yet by this project.**

1. **De-energized visual inspection** — see
   [HARDWARE_VALIDATION_CHECKLIST.md](../../HARDWARE_VALIDATION_CHECKLIST.md)
   Step 1. Unplug, wait for capacitor discharge, open the casing,
   photograph `CN6` and surrounding components. No probe, no power, no
   instrument connected.
2. **Resolve the `CN6` pin-layout discrepancy from the photo** — see
   [PROTOCOL.md](../../PROTOCOL.md) §5. This config's `rx_pin` assignments
   are placeholders until this is resolved.
3. **Electrical-isolation verification** — see
   [SAFETY.md](../../SAFETY.md) §2 and
   [HARDWARE_VALIDATION_CHECKLIST.md](../../HARDWARE_VALIDATION_CHECKLIST.md)
   Step 3. With the appliance unplugged and discharged, use an
   isolation-rated meter to directly measure continuity/isolation between
   `CN6`'s GND pin and mains earth/neutral/line. Do not proceed past this
   step on inference alone.
4. **Confirm baud/parity on a real capture, read-only, before trusting any
   decoded value.** 9600/8N1 is confirmed on two real, unrelated reference
   products (see [PROTOCOL.md](../../PROTOCOL.md) §3) — try it first. This
   can be done with the same passive tap hardware once steps 1–3 are
   satisfied.
5. **Build and validate the ESPHome firmware image** — see
   [BUILDING.md](../../BUILDING.md). Confirmed working: compiles cleanly
   against a pinned ESPHome version, and the resolved build config confirms
   no `tx_pin`/polling/write logic exists anywhere. This confirms the code
   is well-formed and structurally passive — it does **not** confirm it
   decodes real Lydos traffic, since no real Lydos capture exists yet.
6. **Bench-test the flashed firmware off the heater first**, feeding it
   synthetic UART traffic (e.g. from a second microcontroller or a
   USB-UART adapter driving known test frames) to confirm the compiled
   firmware behaves as the offline test harness predicts, before ever
   touching the real `CN6` connector.
7. **Only then**, with isolation independently confirmed (step 3) and the
   firmware bench-tested (step 6), connect the two RX taps plus GND
   reference to the real `CN6` connector, powered from an independent
   isolated supply (see [firmware/docs/HARDWARE_BOM.md](HARDWARE_BOM.md)
   §3) — never from the appliance's own rail.

**This document does not authorize any of the above on its own.** Each
step requires your own judgment and, where noted, appropriate equipment and
care — see [SAFETY.md](../../SAFETY.md).
