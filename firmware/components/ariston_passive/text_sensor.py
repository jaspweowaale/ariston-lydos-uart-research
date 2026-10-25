"""text_sensor: platform for ariston_passive -- enum/flag-class and
Evo-only-unconfirmed registers, published as RAW HEX BYTES ONLY.

These are deliberately NOT boolean/enum entities. This investigation's own
analysis of real Velis 80 data (PROTOCOL.md S7) explicitly
searched for and did NOT find consistent ON/OFF byte semantics for power/
eco/anti-legionella/mode -- publishing a confident boolean here would be
inventing a capability the evidence doesn't support, which the user
explicitly ruled out. Every entity below is entity_category: diagnostic.
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC
from . import AristonPassiveComponent

CONF_ARISTON_PASSIVE_ID = "ariston_passive_id"
CONF_REGISTER = "register"

# address -> (default name, tier)
# MATCHED_ENUM: address presence confirmed on our own Velis 80 data, value
#   semantics NOT established (PROTOCOL.md S7).
# UNCONFIRMED: Evo-repo-only, never cross-checked against our own data at all.
KNOWN_RAW_REGISTERS = {
    0x2105: ("Power Register (raw, unconfirmed semantics)", "MATCHED_ENUM"),
    0x2106: ("ECO Register (raw, unconfirmed semantics)", "MATCHED_ENUM"),
    0x210A: ("Anti-legionella Register (raw, unconfirmed semantics)", "MATCHED_ENUM"),
    0x27DD: ("Mode Register (raw, unconfirmed semantics -- P1/P2 support on this unit unestablished)", "MATCHED_ENUM"),
    0x4BC4: ("Heating Register (raw, EXPERIMENTAL -- Evo-only, not cross-checked)", "UNCONFIRMED"),
    0x4BD3: ("Anti-legionella Cycle Register (raw, EXPERIMENTAL -- Evo-only, not cross-checked)", "UNCONFIRMED"),
    0x475E: ("Time-to-Setpoint Register (raw, EXPERIMENTAL -- Evo-only, not cross-checked)", "UNCONFIRMED"),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ARISTON_PASSIVE_ID): cv.use_id(AristonPassiveComponent),
        cv.Required(CONF_REGISTER): cv.one_of(*KNOWN_RAW_REGISTERS.keys(), int=True),
    }
).extend(text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ARISTON_PASSIVE_ID])
    var = await text_sensor.new_text_sensor(config)
    cg.add(hub.register_raw_text_sensor(config[CONF_REGISTER], var))
