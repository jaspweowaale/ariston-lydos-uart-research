# Home Assistant Entity Mapping — Planning Document

This describes what entities `lydos_passive_monitor.yaml` would create once
flashed and connected to ESPHome's Home Assistant integration. It does not
itself configure or modify Home Assistant.

## 1. If you already have Ariston's cloud integration installed

If you use an existing cloud-based Ariston Home Assistant integration (e.g.
one based on Ariston's own NET cloud API), **this project's entities are
designed never to collide with it.** Every entity below is prefixed
`lydos_cn6_` specifically so it coexists, side by side, with independent
history, alongside whatever entities your existing cloud integration
already provides. If you adapt this config, keep that prefix convention —
don't repoint it at an existing entity ID.

## 2. Entities this config creates

| Entity | Register | Tier | Confidence note |
|---|---|---|---|
| `sensor.lydos_cn6_set_temperature` | `0x2D79` | **CONFIRMED** | See [PROTOCOL.md](../../PROTOCOL.md) §4 |
| `sensor.lydos_cn6_set_temperature_alt` | `0x9E71` | **CONFIRMED** | Same field, alternate address — diagnostic cross-check |
| `sensor.lydos_cn6_displayed_temperature` | `0x9E6E` | MATCHED | Closest candidate for "actual water temperature" — not independently confirmed via a controlled experiment the way the setpoint was |
| `sensor.lydos_cn6_tank_temp_avg` + 4 individual tank sensors | `0x1060`, `0x1368`–`0x136B` | MATCHED, experimental | Real sensor-like behavior observed, not tied to a specific named role via controlled experiment |
| `sensor.lydos_cn6_max_temperature` | `0x2D60` | MATCHED, experimental | Address present in both reference products' poll tables; value semantics not independently re-verified |
| `text_sensor.lydos_cn6_power_raw` | `0x2105` | MATCHED, raw only | **Not a boolean.** No consistent power on/off byte semantics were located in available real data. Raw hex only — nothing here claims to tell you if the heater is on or off. |
| `text_sensor.lydos_cn6_eco_raw` | `0x2106` | MATCHED, raw only | Same caveat |
| `text_sensor.lydos_cn6_antilegionella_raw` | `0x210A` | MATCHED, raw only | Same caveat |
| `text_sensor.lydos_cn6_mode_raw` | `0x27DD` | MATCHED, raw only | Same caveat, **plus**: whether a plain Lydos even supports the MAN/P1/P2/P1+P2 schedule modes documented on the Evo reference product is unestablished |
| `text_sensor.lydos_cn6_heating_raw`, `..._antilegionella_cycle_raw`, `..._time_to_temp_raw` | `0x4BC4`, `0x4BD3`, `0x475E` | UNCONFIRMED | Only documented on the Evo reference product, never independently cross-checked. Marked EXPERIMENTAL, `entity_category: diagnostic`. |

## 3. What this explicitly does NOT include

- No P1/P2 schedule select entity — would require assuming a feature set
  that's unestablished for the Lydos. If confirmed later, a real `select:`
  entity could be added then, not invented now.
- No write-capable entity of any kind — this project is read-only by
  design (see [firmware/lydos_passive_monitor.yaml](../lydos_passive_monitor.yaml)
  header).
- No anti-calc, beep, custom schedule, shower-counter, or anti-bacteria
  entities — the Velis Evo reference implementation confirms these aren't
  exposed over this UART link at all on that product (they require a
  physical button long-press); no evidence suggests the Lydos differs.

## 4. Diagnostic entity categorization

Every register not in the CONFIRMED tier is marked `entity_category:
diagnostic` and/or carries an explicit "(experimental)"/"(raw, unconfirmed
semantics)" qualifier in its friendly name. Unverified mappings are never
exposed as authoritative values — a dashboard built on this data should
visually distinguish the CONFIRMED-tier temperature sensors from everything
else.
