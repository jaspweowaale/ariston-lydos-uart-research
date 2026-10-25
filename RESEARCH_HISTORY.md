# Research History

How this project actually got here — including the dead ends, not just the
result. Written from the project's own working history; nothing below is
invented, and anything uncertain is flagged as such rather than smoothed
over.

## Starting point

This started from a simpler problem: an Ariston VELIS Lydos water heater,
already integrated into a cloud-based home automation setup, with no local
or offline access path at all. The cloud integration worked, but depended
entirely on Ariston's own servers — any cloud outage meant losing telemetry
and control with no fallback. That motivated the underlying question this
whole project exists to answer: can this appliance be read locally, over
its own internal wiring, without touching the cloud at all?

## First real progress: a different product, reverse-engineered from scratch

There was no existing public decode of the Lydos's own UART traffic to
start from. What did exist, from a public electronics forum, was a
teardown-and-capture effort on a **different Ariston product** — the Velis
80 Wi-Fi electric water heater — by independent researchers who had
captured real UART traffic between that unit's ESP32 Wi-Fi module and its
main controller board.

Working from that forum's real capture files, this project independently
re-derived the frame structure from scratch: a `0xC3`-prefixed header,
an ID/command/length/payload/checksum shape, and a simple sum-mod-256
checksum. Re-verifying this against the real captured bytes (not just
trusting the forum's own prose description) produced 2,675 decoded frames,
all checksum-valid. A controlled experiment — changing the unit's target
temperature by a known amount at a known time and watching for a field that
stepped cleanly in response — located and confirmed one real field: the DHW
(domestic hot water) setpoint, at register address `0x2D79`, encoded as an
unsigned 16-bit little-endian value equal to the temperature in °C times
ten. Several other fields (a cluster of temperature-sensor-shaped registers)
were flagged as plausible but not confirmed with the same rigor, and are
reported as such rather than guessed into certainty.

This established the protocol *mechanics* — but on the wrong product. The
Velis 80 is a related but different unit from the Lydos this project
actually targets, and its confirmed baud rate (9600) didn't match the one
forum comment describing the Lydos's own internal link (19200, even
parity) — so none of this directly proved anything about the Lydos itself
yet.

## The hardware-platform hypothesis — real evidence, but not protocol evidence

A separate line of investigation asked a different question: does Ariston
reuse the same Wi-Fi gateway hardware and firmware across its product
range, such that evidence from one product might transfer to others? This
produced real findings — a firmware boot string on the Velis 80
self-identifying as part of a module family (`ATG_REM4A_EWH`), and
Ariston's own spare-parts catalog selling a Wi-Fi daughterboard for the
Lydos *Hybrid* line under a matching retail name — but every one of these
findings was at the level of part numbers and firmware naming, never actual
wire-protocol bytes. This hypothesis made the *platform-sharing* case
stronger over several passes, but never once answered the actual question
this project needed answered: does the wire protocol itself transfer.

## A purchase plan, designed and then abandoned

Given the platform-sharing evidence, one path considered in depth was
purchasing a standalone retail Wi-Fi daughterboard (sold as a Lydos Hybrid
spare part) and bench-testing it in isolation, with a software-simulated
"fake main controller" built from the already-decoded Velis 80 data to try
to keep the purchased board producing real traffic instead of sitting in a
boot-retry loop waiting for a controller that was never connected. This was
designed carefully — including identifying a real, previously unflagged
risk that a standalone board might never produce usable bus traffic at all
without a believable controller present — but it was a plan for spending
money and bench time on an board from a *different sub-line* (Hybrid, a
heat-pump variant) than the actual target unit (plain electric Lydos),
testing the platform hypothesis in the abstract rather than the actual
appliance. This direction was explicitly abandoned in favor of first
establishing exactly what hardware the actual target unit contains, before
buying anything.

## A security-research detour that correctly ruled itself out

With explicit authorization to research the owned device's own gateway,
one candidate approach investigated was intercepting the gateway's cloud
MQTT connection via a DNS redirect to a replacement broker — a published,
independent open-source project implements exactly this. Investigating it
found that its mechanism depends on the device accepting a self-signed
certificate after being redirected — and this project's own earlier live
network capture of the actual target gateway had already shown its
firmware performs full TLS certificate validation and pinning, rejecting
exactly this kind of redirect. The cloud-interception approach was
correctly ruled out as not viable against this device, rather than pursued
further on hope. The same research pass found the cloud backend runs on a
named third-party commercial IoT platform and identified a local Wi-Fi
provisioning window on the gateway, but no further exploitable local
network service.

## The actual breakthrough: someone else had already solved this, for a different model

The same research pass that investigated the broker approach also searched
more recent pages of the original forum thread — and found a link to a
**separate, independent, working, open-source project**: a complete
ESPHome-based local UART controller for yet another Ariston model (the
Velis Evo Plus 100 EU), built by a different author, achieving full local
read *and write* control over that unit's own `CN6` connector, with no
cloud dependency at all, confirmed working on real hardware.

Comparing that project's own published register map against this project's
already-decoded Velis 80 data found **16 exact register-address matches**
— not a coincidence at this scale — including one complete request/response
exchange (an initialization handshake) that matched byte-for-byte, checksum
included, across both independently-built implementations on two different
real products. This was the first genuine byte-level cross-model protocol
evidence found in the whole investigation, and it immediately made the
REM4-board-purchase and controller-simulation lines of work obsolete: there
was now a real, proven, working template to adapt, rather than a
from-scratch reverse-engineering problem or a bench experiment on the wrong
sub-line of hardware.

## A design mistake, caught before any hardware was involved

The first attempt to adapt that working project directly copied its
architecture — including the part where its ESP32 actively transmits read
commands to the heater on a repeating timer. That architecture made sense
for the Evo Plus project, because that unit's `CN6` connector was
*unpopulated* — no Wi-Fi module was ever attached to it, so there was
nothing for a new, actively-transmitting ESP32 to collide with.

**This project's actual target heater is different: its `CN6`-equivalent
connector already has an active, in-service Wi-Fi gateway attached and
running, right now.** UART is a point-to-point electrical link, not a
shared bus — a second device actively transmitting on the same wires as an
existing, already-functioning gateway would contend with it. This was
caught during design review, before anything was built against the
mismatch, and it is the direct reason this project's design is **two
independent, receive-only UART inputs with zero transmit capability**,
rather than a copy of the working write-capable reference design: a
passive eavesdropper on the existing gateway's own ongoing conversation,
not a second device trying to also be the bus master.

## What was actually built and verified from there

The corrected, passive-only design was implemented as a framework-agnostic
C++ decoder (frame synchronization across two independent streams,
checksum validation, request/response correlation, register-tier-gated
publishing, and duplicate suppression), with an ESPHome wrapper around it.
It was offline-tested against the real 2,675-frame Velis 80 dataset
(9 checks, all passing at the time) and then compiled end-to-end through a
real, pinned ESPHome toolchain install in an isolated environment — not
just validated as source code, but actually built into a working firmware
image, with two real build-configuration issues found and fixed in the
process. The resolved, compiled configuration was directly inspected (not
just the source YAML) to confirm no transmit pin exists on either UART bus
and no polling/write logic survived into the actual build.

## Where this stands now

Everything above demonstrates the *software* is sound against the data and
tooling available. It does not, and cannot, prove this works against a
real Lydos — nobody has photographed this unit's actual `CN6`-equivalent
connector, confirmed its pin layout, measured its real baud rate and
parity, or verified its electrical isolation from mains. This project is
now being prepared for public release specifically to get real Lydos owners
involved in closing those remaining, physical-world gaps — see
[HARDWARE_VALIDATION_CHECKLIST.md](HARDWARE_VALIDATION_CHECKLIST.md).
