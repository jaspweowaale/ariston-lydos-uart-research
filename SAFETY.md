# Safety — Read Before Any Physical Work

**This project does not warrant or guarantee electrical safety. If you open,
probe, or connect anything to your appliance, you are doing so entirely at
your own risk.** Ariston water heaters are mains-powered appliances. The
following is general guidance based on this project's own research, not a
substitute for your own judgment, local electrical code, or a qualified
technician.

## 1. The appliance is mains-powered, even "off"

Unplugging the appliance removes power to the control board, but internal
capacitors can retain a charge for some time afterward. **Wait several
minutes after unplugging before opening the casing**, to allow capacitors to
discharge, before touching anything inside.

## 2. Isolation from mains is NOT independently confirmed

This project's research found circumstantial, suggestive evidence (general
appliance-safety regulatory expectations for user-touchable control
surfaces; a related product's known isolated power-supply topology) that
the low-voltage control-board domain — including the `CN6` connector this
project targets — is likely isolated from mains voltage. **This is an
inference, not a measurement, and has not been independently confirmed for
any specific Lydos unit.**

Before connecting any measurement or monitoring equipment to a live
(powered) appliance:

1. With the appliance **unplugged and discharged**, use an **isolation-rated
   multimeter** (CAT II/III or better) to directly measure continuity and
   isolation between the target connector's ground reference and the
   appliance's mains earth/neutral/line connections.
2. Do not proceed to a live connection if this measurement is inconclusive
   or you're not confident interpreting it. Consider having a qualified
   electrician or appliance technician perform or verify this check.
3. **A visual inspection alone — even one that photographs a transformer or
   isolated-looking power supply component — does not by itself establish
   isolation.** It narrows uncertainty; it does not close it. Only a direct
   measurement does.

## 3. Even a "receive-only" connection is a real electrical connection

This project's firmware design never transmits onto the UART bus — but any
monitoring hardware still needs a shared ground reference with the
appliance's control board for the received signals to be meaningful at all.
**That ground-reference tap is a real electrical connection to the
appliance**, separate from (and no safer by default than) drawing power
from it. Treat it with the same caution as any other physical connection
until isolation is independently confirmed per §2.

## 4. Do not power monitoring hardware from the appliance's own supply

Even if a connector's data pins are confirmed safe, **do not power any
external monitoring device (e.g. an ESP32 board) from the appliance's own
internal low-voltage rail** until isolation is independently confirmed.
Power any such device from a separate, independent, isolated source (a USB
power bank, which has no mains connection at all, or a mains-isolated USB
wall adapter) instead.

## 5. Defense in depth for any live connection

Once isolation is confirmed and you choose to proceed:

- Use a **USB isolator** between any computer/analyzer and the appliance,
  even after isolation is confirmed, as a second layer of protection.
- Keep any laptop used for monitoring on battery power during a live
  session, not plugged into mains-charging, to avoid ground-loop/potential
  differences between the laptop's earth reference and the appliance.
- Prefer non-destructive connection methods (e.g. thin wires through
  existing connector holes) over soldering, and disconnect the appliance
  from mains while making or breaking any physical connection.

## 6. This is your appliance and your decision

Nothing in this repository has been tested against a real Ariston Lydos.
Opening a mains-powered appliance, probing its internal electronics, or
connecting external hardware to it can cause electric shock, fire, damage to
the appliance, or voided warranty/insurance coverage. If you are not
confident performing electrical safety verification yourself, consult a
qualified technician before doing any of this. The maintainers and
contributors to this project accept no liability for damage, injury, or loss
resulting from its use.
