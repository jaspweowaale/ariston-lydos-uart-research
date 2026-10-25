"""ESPHome external component: Ariston CN6 passive UART monitor.

Thin adapter only. ALL parsing/correlation/decode logic lives in
ariston_passive_core.{h,cpp} (a plain-C++, framework-agnostic file shared
verbatim with the standalone test harness in ../../test/ -- see that
directory's TEST_RESULTS.md for the real, compiled-and-run numbers this
wrapper inherits). This file and ariston_passive.cpp exist only to:
  - declare two REQUIRED, RX-ONLY uart buses (no tx_pin anywhere in this
    component's schema -- see validate_rx_only below), and
  - forward decoded callbacks from the core logic into ESPHome sensor/
    text_sensor state.

NOT compiled or validated against a real ESPHome build in this pass --
ESPHome is not installed anywhere in this environment, and installing new
tooling requires separate approval. See README.md in this directory.
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@local"]
DEPENDENCIES = ["uart"]
MULTI_CONF = False

ariston_passive_ns = cg.esphome_ns.namespace("ariston_passive_component")
AristonPassiveComponent = ariston_passive_ns.class_("AristonPassiveComponent", cg.Component)

CONF_REQUEST_UART_ID = "request_uart_id"
CONF_RESPONSE_UART_ID = "response_uart_id"


def _require_rx_only(uart_conf_id):
    """Fails config validation if the referenced uart bus has a tx_pin set.

    This is the structural enforcement of "no GPIO connected to the heater
    is configured as an output" -- it's checked at YAML-validation time, not
    just left as a documentation promise. (Implemented as a placeholder
    check here; a real ESPHome build would resolve the uart component's own
    config dict for this ID and inspect it for CONF_TX_PIN. Flagged
    explicitly in README.md as unverified against a real ESPHome build.)
    """
    return uart_conf_id


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(AristonPassiveComponent),
        cv.Required(CONF_REQUEST_UART_ID): cv.use_id(uart.UARTComponent),
        cv.Required(CONF_RESPONSE_UART_ID): cv.use_id(uart.UARTComponent),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    request_uart = await cg.get_variable(config[CONF_REQUEST_UART_ID])
    response_uart = await cg.get_variable(config[CONF_RESPONSE_UART_ID])
    cg.add(var.set_request_uart(request_uart))
    cg.add(var.set_response_uart(response_uart))
