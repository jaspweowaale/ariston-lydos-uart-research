# Contributing

Thanks for your interest. This project is at an early, pre-hardware-testing
stage — see [README.md](README.md) for current status.

## Ways to contribute right now, without any hardware

- **Read [PROTOCOL.md](PROTOCOL.md) and [firmware/docs/](firmware/docs/)**
  and point out anything that looks wrong, inconsistent, or
  overconfident given the cited evidence. Confidence-tier accuracy matters
  more than completeness in this project.
- Review `firmware/core/` and `test/` for logic issues — the decode/
  correlation code is the part of this project with the most real testing
  behind it, and the part most worth scrutinizing further.
- Improve the offline test harness (`test/test_harness.cpp`) with more edge
  cases, especially ones not already covered (see `test/README.md` for what
  is and isn't tested today).

## Ways to contribute with a real Lydos, no live connection required

See [HARDWARE_VALIDATION_CHECKLIST.md](HARDWARE_VALIDATION_CHECKLIST.md).
De-energized photos of your own unit's control board are the single most
valuable contribution possible right now.

## Ways to contribute with a live connection

Please read [SAFETY.md](SAFETY.md) in full first. If you capture real Lydos
UART traffic (passively, read-only), please:

- Report aggregate findings (does 9600/8N1 framing decode cleanly? what
  checksum-valid frame count did you get?) rather than assuming you need to
  publish raw capture files.
- If you do want to share a raw capture, open an issue first to discuss —
  this project deliberately does not bundle any third-party or
  personally-identifiable capture data (see [README.md](README.md)), and
  wants to keep that policy consistent for new contributions too. Strip
  anything that could identify you personally (exact address, account
  info, etc.) if your capture happens to include it.

## Pull requests

- Keep changes to `firmware/core/` and
  `firmware/components/ariston_passive/` in sync (see that directory's
  README for why two copies currently exist).
- If you change decode logic, re-run the offline test harness and include
  the updated pass/fail output in your PR description.
- If you change anything safety-related (power/isolation guidance, the
  receive-only architecture), please over-explain your reasoning — this
  project would rather be slow and correct than fast and wrong about
  anything that touches a mains appliance.

## Code of conduct

Be respectful. This is a hobbyist reverse-engineering project about a
consumer appliance, not a security research target in the adversarial
sense — please keep discussion constructive and appliance-owner-focused.
