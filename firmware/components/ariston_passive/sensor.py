"""sensor: platform for ariston_passive -- temperature-class registers only
(CONFIRMED and MATCHED_TEMP tiers in ariston_passive_core.h's RegTier enum).
Enum/flag-class registers (power/eco/anti-legionella/mode) are NOT exposed
here -- see text_sensor.py, raw bytes only, per PROTOCOL.md
S7's documented "NOT LOCATED, inconclusive" finding for their byte semantics.
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from . import AristonPassiveComponent, ariston_passive_ns

CONF_ARISTON_PASSIVE_ID = "ariston_passive_id"
CONF_REGISTER = "register"

AddTemperatureSensorAction = ariston_passive_ns.class_("AddTemperatureSensorAction")

# address -> (default name, tier, default entity_category)
# Tier annotations match core/ariston_passive_core.cpp's kKnownRegisters table
# exactly -- do not add an address here that isn't also present there.
KNOWN_TEMP_REGISTERS = {
    0x2D79: ("DHW Target Temperature", "CONFIRMED", None),
    0x9E71: ("DHW Target Temperature (alt address)", "CONFIRMED", ENTITY_CATEGORY_DIAGNOSTIC),
    0x9E6E: ("Displayed Temperature", "MATCHED", None),
    0x1060: ("Tank Temperature (avg, experimental)", "MATCHED", ENTITY_CATEGORY_DIAGNOSTIC),
    0x1368: ("Tank Temperature Right Bottom (experimental)", "MATCHED", ENTITY_CATEGORY_DIAGNOSTIC),
    0x1369: ("Tank Temperature Right Top (experimental)", "MATCHED", ENTITY_CATEGORY_DIAGNOSTIC),
    0x136A: ("Tank Temperature Left Bottom (experimental)", "MATCHED", ENTITY_CATEGORY_DIAGNOSTIC),
    0x136B: ("Tank Temperature Left Top (experimental)", "MATCHED", ENTITY_CATEGORY_DIAGNOSTIC),
    0x2D60: ("Max Temperature Limit (experimental)", "MATCHED", ENTITY_CATEGORY_DIAGNOSTIC),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ARISTON_PASSIVE_ID): cv.use_id(AristonPassiveComponent),
        cv.Required(CONF_REGISTER): cv.one_of(*KNOWN_TEMP_REGISTERS.keys(), int=True),
    }
).extend(sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
    accuracy_decimals=1,
))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ARISTON_PASSIVE_ID])
    var = await sensor.new_sensor(config)
    cg.add(hub.register_temperature_sensor(config[CONF_REGISTER], var))
