# Hardware Bill of Materials — Passive, Two-Direction Monitoring

**Nothing here has been purchased or tested.** This is a BOM for a
receive-only hardware design: every connection to `CN6` is an input, there
is no transmit line to drive and no risk of contending with the heater's
existing Wi-Fi gateway.

## 1. ESP32 variant

Minimum requirement: 2 UART-capable RX inputs on separate hardware UART
peripherals, each configurable RX-only (no paired TX needed).

- **Suggested: ESP32-S3** (e.g. a generic DevKitC-1 board) — multiple
  independent hardware UART peripherals, each configurable RX-only in
  ESPHome's `uart:` component. Mature, well-supported, inexpensive.
- Most other ESP32 variants (original ESP32, S2, C3, C6) also have at least
  2 usable UART peripherals and would work equally well for a pure-RX use
  case. If you already own a different ESP32 board, it likely works too —
  the S3 is a reasonable default, not the only option.

## 2. Voltage compatibility

`CN6`'s UART signaling is 5V logic (confirmed on both reference products in
[PROTOCOL.md](../../PROTOCOL.md)). Most ESP32 GPIO is 3.3V-tolerant on input
up to roughly 3.6V, not reliably safe at a continuous 5V logic high — a
simple resistor voltage divider (two resistors per line) or a basic
logic-level shifter board is needed on each RX input. Because these are
RX-only, a passive voltage divider (e.g. ~10kΩ/20kΩ bringing 5V down to
~3.3V) is sufficient — no bidirectional level-shifter IC is required.

## 3. Electrical isolation — hard requirement, not a suggestion

**Do not power the ESP32 from `CN6`'s own +5V rail.** See
[SAFETY.md](../../SAFETY.md) for why this project treats isolation as
unconfirmed and what to verify before any live connection.

- Power the ESP32 from a separate, independent, isolated source — a USB
  power bank (no mains connection at all) or a mains-isolated USB wall
  adapter.
- A shared ground reference with `CN6`'s GND pin is still electrically
  necessary for the RX signals to be meaningful at all — this is a real
  connection to the appliance, separate from drawing power from it, and is
  exactly the kind of connection the isolation check in
  [SAFETY.md](../../SAFETY.md) exists to clear before it's made.

## 4. Connector / wiring

Non-destructive tap only — thin wires through a connector's existing holes,
no soldering. Three connections needed: two RX signal taps (one per
direction) plus the shared GND reference above. No connection to `CN6`'s
own +5V pin is needed or recommended (see §3).

## 5. Approximate costs (rough ranges, nothing purchased)

| Item | Purpose | Approx. cost (USD) |
|---|---|---|
| ESP32-S3 dev board (or an equivalent board you already own) | Main controller, 2 independent RX-only UART inputs | $8–15 |
| 2x passive voltage-divider resistor pairs, or 1 basic 2-channel level shifter module | 5V→3.3V-safe signal levels on both RX lines | $2–8 |
| Isolated USB power source (power bank, or isolated USB wall adapter) | Powers the ESP32 independently of the appliance — hard requirement, §3 | $10–20 |
| Insulated clip leads / push-pins | Non-destructive tap, no soldering | $5–15 |

**Rough total: ~$25–60.**
