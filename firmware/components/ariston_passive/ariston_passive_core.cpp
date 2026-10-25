// PACKAGED COPY for ESPHome external_components build (ESPHome does not pull in
// a sibling core/ dir outside components/<name>/). Canonical source of truth is
// ../../core/ariston_passive_core.cpp  -- keep both in sync if the decode logic changes;
// test/TEST_RESULTS.md validates the canonical copy, not this one directly.
#include "ariston_passive_core.h"

namespace ariston_passive {

const std::vector<RegInfo> kKnownRegisters = {
    {0x2D79, RegTier::CONFIRMED, "set_temperature"},
    {0x9E71, RegTier::CONFIRMED, "set_temperature_alt"},
    {0x9E6E, RegTier::MATCHED_TEMP, "displayed_temperature"},
    {0x1060, RegTier::MATCHED_TEMP, "tank_temp_avg"},
    {0x1368, RegTier::MATCHED_TEMP, "tank_temp_right_bottom"},
    {0x1369, RegTier::MATCHED_TEMP, "tank_temp_right_top"},
    {0x136A, RegTier::MATCHED_TEMP, "tank_temp_left_bottom"},
    {0x136B, RegTier::MATCHED_TEMP, "tank_temp_left_top"},
    {0x2D60, RegTier::MATCHED_TEMP, "max_temperature"},
    {0x2105, RegTier::MATCHED_ENUM, "power_raw"},
    {0x2106, RegTier::MATCHED_ENUM, "eco_raw"},
    {0x210A, RegTier::MATCHED_ENUM, "antilegionella_raw"},
    {0x27DD, RegTier::MATCHED_ENUM, "mode_raw"},
    {0x4BC4, RegTier::UNCONFIRMED, "heating_raw"},
    {0x4BD3, RegTier::UNCONFIRMED, "antilegionella_cycle_raw"},
    {0x475E, RegTier::UNCONFIRMED, "time_to_temp_raw"},
    {0xD84B, RegTier::UNDECODED, "undecoded_d84b"},
    {0xD95E, RegTier::UNDECODED, "undecoded_d95e"},
    {0xD95F, RegTier::UNDECODED, "undecoded_d95f"},
    {0xDC44, RegTier::UNDECODED, "undecoded_dc44"},
};

const RegInfo *lookup_register(uint16_t addr) {
  for (auto &r : kKnownRegisters)
    if (r.addr == addr) return &r;
  return nullptr;
}

static const char *tier_name(RegTier t) {
  switch (t) {
    case RegTier::CONFIRMED:
      return "CONFIRMED";
    case RegTier::MATCHED_TEMP:
    case RegTier::MATCHED_ENUM:
      return "MATCHED";
    case RegTier::UNCONFIRMED:
      return "UNCONFIRMED";
    default:
      return "UNDECODED";
  }
}

void FrameSync::reset() {
  state_ = State::WAIT_HEADER;
  buf_.clear();
}

bool FrameSync::feed(uint8_t byte, uint32_t now_ms, Frame &out) {
  if (state_ != State::WAIT_HEADER && (now_ms - frame_start_ms_) > FRAME_TIMEOUT_MS) {
    reset();  // interrupted mid-frame for too long -- abandon and resync
  }

  switch (state_) {
    case State::WAIT_HEADER:
      if (byte == LEADIN_BYTE) {
        state_ = State::WAIT_LEADIN_CONFIRM;
        return false;
      }
      if (byte == HEADER_BYTE) {
        buf_.clear();
        buf_.push_back(byte);
        frame_start_ms_ = now_ms;
        state_ = State::READ_ID;
      }
      // anything else: garbage between frames, silently ignored
      return false;

    case State::WAIT_LEADIN_CONFIRM:
      if (byte == HEADER_BYTE) {
        buf_.clear();
        buf_.push_back(byte);
        frame_start_ms_ = now_ms;
        state_ = State::READ_ID;
        return false;
      }
      // the lead-in byte wasn't actually followed by a real header;
      // drop it and re-evaluate this byte as a potential header itself.
      state_ = State::WAIT_HEADER;
      return feed(byte, now_ms, out);

    case State::READ_ID:
      buf_.push_back(byte);
      state_ = State::READ_CMD;
      return false;

    case State::READ_CMD:
      buf_.push_back(byte);
      state_ = State::READ_LEN;
      return false;

    case State::READ_LEN:
      if (byte > MAX_PAYLOAD_LEN) {
        // Implausible length -- this was never really a frame header.
        // Resync and re-offer this same byte as a potential new header.
        reset();
        return feed(byte, now_ms, out);
      }
      buf_.push_back(byte);
      state_ = (byte == 0) ? State::READ_CHK : State::READ_PAYLOAD;
      return false;

    case State::READ_PAYLOAD:
      buf_.push_back(byte);
      if (buf_.size() == (size_t)(4 + buf_[3])) {
        state_ = State::READ_CHK;
      }
      return false;

    case State::READ_CHK: {
      buf_.push_back(byte);
      uint8_t sum = 0;
      for (size_t i = 0; i + 1 < buf_.size(); ++i) sum += buf_[i];
      out.id = buf_[1];
      out.cmd = buf_[2];
      out.len = buf_[3];
      out.payload.assign(buf_.begin() + 4, buf_.end() - 1);
      out.chk = buf_.back();
      out.calc = sum;
      out.valid = (sum == out.chk);
      out.timestamp_ms = frame_start_ms_;
      out.raw = buf_;
      reset();
      return true;
    }
  }
  return false;
}

void PassiveMonitor::feed_request_byte(uint8_t b, uint32_t now_ms) {
  Frame f;
  if (!request_sync_.feed(b, now_ms, f)) return;
  request_frames_seen_++;
  if (f.valid) request_frames_valid_++;
  if (f.valid && f.raw == last_request_raw_ && (now_ms - last_request_ms_) < DUPLICATE_SUPPRESS_MS) {
    return;  // identical back-to-back frame inside an implausibly short window -- parse artifact, not a new transmission
  }
  last_request_raw_ = f.raw;
  last_request_ms_ = now_ms;
  if (on_request_frame) on_request_frame(f);
  if (f.valid) handle_request_frame(f);
}

void PassiveMonitor::feed_response_byte(uint8_t b, uint32_t now_ms) {
  Frame f;
  if (!response_sync_.feed(b, now_ms, f)) return;
  response_frames_seen_++;
  if (f.valid) response_frames_valid_++;
  if (f.valid && f.raw == last_response_raw_ && (now_ms - last_response_ms_) < DUPLICATE_SUPPRESS_MS) {
    return;
  }
  last_response_raw_ = f.raw;
  last_response_ms_ = now_ms;
  if (on_response_frame) on_response_frame(f);
  if (f.valid) handle_response_frame(f);
}

void PassiveMonitor::handle_request_frame(const Frame &f) {
  if (f.cmd == 0x25 && f.payload.size() == 2) {
    pending_.active = true;
    pending_.cmd = 0x25;
    pending_.addresses = {(uint16_t)(f.payload[0] | (f.payload[1] << 8))};
    pending_.timestamp_ms = f.timestamp_ms;
  } else if (f.cmd == 0x23 && f.payload.size() >= 2 && f.payload.size() % 2 == 0) {
    pending_.active = true;
    pending_.cmd = 0x23;
    pending_.addresses.clear();
    for (size_t i = 0; i + 1 < f.payload.size(); i += 2)
      pending_.addresses.push_back((uint16_t)(f.payload[i] | (f.payload[i + 1] << 8)));
    pending_.timestamp_ms = f.timestamp_ms;
  } else if (f.cmd == 0x33 && f.payload.size() >= 2) {
    // A write issued by the EXISTING factory gateway -- decode directly from
    // the request frame itself (mirrors DeDaMrAz's own forum example
    // C3 41 33 04 79 2D B2 02 95). No response pairing needed for this.
    uint16_t addr = (uint16_t)(f.payload[0] | (f.payload[1] << 8));
    std::vector<uint8_t> value(f.payload.begin() + 2, f.payload.end());
    publish_decoded(addr, value);
    pending_.active = false;
  } else {
    // 0x52 handshake or anything unrecognised -- nothing to correlate.
    pending_.active = false;
  }
}

void PassiveMonitor::handle_response_frame(const Frame &f) {
  if (!pending_.active) {
    if (on_unmatched_response) on_unmatched_response();
    return;
  }
  if ((f.timestamp_ms - pending_.timestamp_ms) > CORRELATION_WINDOW_MS) {
    pending_.active = false;  // too stale to trust -- drop rather than mispair
    if (on_unmatched_response) on_unmatched_response();
    return;
  }

  if (pending_.cmd == 0x25 && pending_.addresses.size() == 1) {
    if (f.payload.size() >= 2) {
      std::vector<uint8_t> value(f.payload.begin(), f.payload.begin() + 2);
      publish_decoded(pending_.addresses[0], value);
      correlated_pairs_++;
    }
  } else if (pending_.cmd == 0x23) {
    size_t n = pending_.addresses.size();
    if (f.payload.size() == n * 2) {
      for (size_t i = 0; i < n; ++i) {
        std::vector<uint8_t> value(f.payload.begin() + i * 2, f.payload.begin() + i * 2 + 2);
        publish_decoded(pending_.addresses[i], value);
      }
      correlated_pairs_++;
    }
    // else: short/misaligned reply -- discarded. PROTOCOL.md
    // S2 documents this as expected for ~50-60% of 0x23 replies in real
    // captures; zip-aligning a short reply can't be trusted, so this is
    // matching established methodology, not a bug.
  }
  pending_.active = false;
}

void PassiveMonitor::publish_decoded(uint16_t addr, const std::vector<uint8_t> &value_bytes) {
  const RegInfo *info = lookup_register(addr);
  if (!info) return;  // not a register we track -- ignore
  if (info->tier == RegTier::UNDECODED) return;  // polled, but nobody has assigned this a meaning

  if (info->tier == RegTier::CONFIRMED || info->tier == RegTier::MATCHED_TEMP) {
    if (value_bytes.size() < 2) return;
    uint16_t raw = (uint16_t)(value_bytes[0] | (value_bytes[1] << 8));
    auto it = last_temp_raw_.find(addr);
    if (it != last_temp_raw_.end() && it->second == raw) return;  // dedupe: no real change
    last_temp_raw_[addr] = raw;
    if (on_decoded_temperature) on_decoded_temperature(addr, raw / 10.0, tier_name(info->tier));
    return;
  }

  // MATCHED_ENUM / UNCONFIRMED -- raw bytes only, semantics not established.
  auto it = last_enum_raw_.find(addr);
  if (it != last_enum_raw_.end() && it->second == value_bytes) return;
  last_enum_raw_[addr] = value_bytes;
  if (on_decoded_raw) on_decoded_raw(addr, value_bytes, tier_name(info->tier));
}

}  // namespace ariston_passive
