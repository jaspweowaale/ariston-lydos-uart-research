# Outstanding Compatibility and Electrical-Safety Questions

Nothing below is resolved by this project's software work — building and
testing the decoder does not, by itself, answer any of these.

## Compatibility

1. **Baud/parity unconfirmed for the Lydos specifically.** 9600/8N1 is
   confirmed on two real, independently-built, independently-published
   products. The Lydos's own claimed 19200/even-parity rests on one
   unverified forum comment, never byte-verified, and not even confirmed to
   describe the same connector referenced in [PROTOCOL.md](../../PROTOCOL.md)
   §5. This remains the single largest open unknown.
2. **`CN6` pin-layout discrepancy, unresolved.** See
   [PROTOCOL.md](../../PROTOCOL.md) §5 — needs a photo (Step 1 of
   [INSTALLATION_PREREQUISITES.md](INSTALLATION_PREREQUISITES.md)) to
   resolve.
3. **Register-map divergence risk, smaller than feared but not zero.** 16
   exact address matches between the two reference products is strong, but
   they already diverge on roughly a dozen addresses each has that the
   other's own documentation doesn't cover. The Lydos could have its own
   unique subset on top of that.
4. **The Lydos's own feature set is not established** — whether it
   supports P1/P2 scheduling, has the same number of tank thermistors as
   the Evo reference product, or differs in any other capability has never
   been checked against a physical unit's own panel menu.
5. **Existing-gateway coexistence is reasoned, not tested.** The read-only,
   dual-RX architecture here is based on standard UART electrical norms (a
   high-impedance listen-only input doesn't load or contend with an
   existing transmitter) — a well-founded inference, not something
   verified against any Ariston unit with an already-populated `CN6`, by
   this project or (as far as its research found) anyone publicly.

## Electrical safety

6. **`CN6` isolation-from-mains status is still unconfirmed for the
   Lydos.** See [SAFETY.md](../../SAFETY.md) §2 — the circumstantial case
   is suggestive, not a measurement.
7. **No photographic confirmation `CN6` exists on a plain Lydos at all**,
   in this exact form, location, or pin count.
8. **The GND-reference-tap nuance** (see
   [HARDWARE_BOM.md](HARDWARE_BOM.md) §3): even a purely receive-only
   connection still requires a shared ground reference with the heater's
   board for the signals to be meaningful — a real electrical connection to
   the heater, separate from drawing power from it, and exactly the kind of
   connection the isolation check in [SAFETY.md](../../SAFETY.md) exists to
   clear before it's made.

## What this project's software work does and does not change

Building and offline-testing the decoder (19/19 tests passed against a
fully synthetic, fabricated dataset shipped in this repo; 9/9 passed
historically against real reference-product data not included here — see
[test/README.md](../../test/README.md)) demonstrates the *logic* is sound.
It does not, and cannot by its nature as offline software testing, touch
any of the 8 items above. All 8 remain exactly where they are regardless of
how well the software performs offline, and every one of them gates
physical installation.
