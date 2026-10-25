#include "ariston_passive.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ariston_passive_component {

static const char *const TAG = "ariston_passive";

void AristonPassiveComponent::setup() {
  monitor_.on_decoded_temperature = [this](uint16_t addr, double value_c, const char *tier) {
    auto it = temp_sensors_.find(addr);
    if (it != temp_sensors_.end() && it->second != nullptr) {
      it->second->publish_state(value_c);
    }
    ESP_LOGD(TAG, "temperature 0x%04X = %.1fC [%s]", addr, value_c, tier);
  };
  monitor_.on_decoded_raw = [this](uint16_t addr, const std::vector<uint8_t> &raw, const char *tier) {
    auto it = raw_sensors_.find(addr);
    if (it != raw_sensors_.end() && it->second != nullptr) {
      char buf[8];
      snprintf(buf, sizeof(buf), "%02X%02X", raw.size() > 0 ? raw[0] : 0, raw.size() > 1 ? raw[1] : 0);
      it->second->publish_state(std::string(buf));
    }
    ESP_LOGD(TAG, "raw 0x%04X = %02X %02X [%s, semantics unconfirmed]", addr,
             raw.size() > 0 ? raw[0] : 0, raw.size() > 1 ? raw[1] : 0, tier);
  };
  monitor_.on_unmatched_response = [this]() { unmatched_response_count_++; };
}

void AristonPassiveComponent::loop() {
  // Receive-only. Neither branch below ever calls a write method on either
  // uart bus -- this loop only drains bytes that already arrived on the RX
  // line into the core parser. See file header invariant.
  uint32_t now = millis();

  if (request_uart_ != nullptr) {
    uint8_t b;
    while (request_uart_->available() > 0) {
      if (request_uart_->read_byte(&b)) monitor_.feed_request_byte(b, now);
    }
  }
  if (response_uart_ != nullptr) {
    uint8_t b;
    while (response_uart_->available() > 0) {
      if (response_uart_->read_byte(&b)) monitor_.feed_response_byte(b, now);
    }
  }
}

void AristonPassiveComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Ariston Passive CN6 Monitor:");
  ESP_LOGCONFIG(TAG, "  request frames seen/valid: %u/%u", (unsigned) monitor_.request_frames_seen(),
                (unsigned) monitor_.request_frames_valid());
  ESP_LOGCONFIG(TAG, "  response frames seen/valid: %u/%u", (unsigned) monitor_.response_frames_seen(),
                (unsigned) monitor_.response_frames_valid());
  ESP_LOGCONFIG(TAG, "  correlated pairs: %u", (unsigned) monitor_.correlated_pairs());
  ESP_LOGCONFIG(TAG, "  unmatched responses: %u", (unsigned) unmatched_response_count_);
}

}  // namespace ariston_passive_component
}  // namespace esphome
