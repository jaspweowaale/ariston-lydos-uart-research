// ESPHome wrapper around ariston_passive_core.h/.cpp.
//
// Design invariant, enforced structurally, not just by convention: this
// class holds references to two uart::UARTComponent instances and only ever
// CALLS read_array()/peek_byte()/available() on them in loop() -- there is
// no write_array()/write_byte() call anywhere in this file or in
// ariston_passive_core.cpp. Grep both files for "write" if verifying this.
#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "ariston_passive_core.h"
#include <map>
#include <string>

namespace esphome {
namespace ariston_passive_component {

class AristonPassiveComponent : public Component {
 public:
  void set_request_uart(uart::UARTComponent *u) { request_uart_ = u; }
  void set_response_uart(uart::UARTComponent *u) { response_uart_ = u; }

  // Called from sensor.py/text_sensor.py codegen for each configured
  // register entity -- maps a register address to the ESPHome entity that
  // should receive its decoded value. Addresses not registered here are
  // still parsed/logged by the core (for diagnostics) but never published.
  void register_temperature_sensor(uint16_t addr, sensor::Sensor *s) { temp_sensors_[addr] = s; }
  void register_raw_text_sensor(uint16_t addr, text_sensor::TextSensor *s) { raw_sensors_[addr] = s; }

  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  uart::UARTComponent *request_uart_{nullptr};
  uart::UARTComponent *response_uart_{nullptr};
  ariston_passive::PassiveMonitor monitor_;

  std::map<uint16_t, sensor::Sensor *> temp_sensors_;
  std::map<uint16_t, text_sensor::TextSensor *> raw_sensors_;

  uint32_t unmatched_response_count_{0};
};

}  // namespace ariston_passive_component
}  // namespace esphome
