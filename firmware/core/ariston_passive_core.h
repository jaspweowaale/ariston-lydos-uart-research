// Ariston passive UART monitor -- framework-agnostic core logic.
//
// This file has zero ESPHome/Arduino dependency on purpose: it is compiled
// and unit-tested standalone with plain g++ against real, already-decoded
// Ariston UART capture data (see ../test/). The ESPHome component in
// ../components/ariston_passive/ is a thin wrapper that forwards UART bytes
// into this code and forwards this code's callbacks into ESPHome sensors --
// it adds no additional parsing/correlation logic of its own, so what gets
// tested here is what actually runs on the device.
//
// Design invariant: this code NEVER writes to a UART. There is no public
// method anywhere in this class that transmits a byte. Two independent,
// receive-only byte streams go IN (feed_request_byte / feed_response_byte);
// decoded values come OUT via callbacks. That is the entire interface.
#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include <map>
#include <string>

namespace ariston_passive {

constexpr uint8_t HEADER_BYTE = 0xC3;
constexpr uint8_t LEADIN_BYTE = 0x3C;        // optional byte occasionally seen before 0xC3 (see decoder_verify.py)
constexpr size_t MAX_PAYLOAD_LEN = 64;       // safety cap -- largest real payload observed is 18 bytes
constexpr uint32_t FRAME_TIMEOUT_MS = 500;   // abandon a stuck in-progress frame after this long (real frames complete in <45ms)
constexpr uint32_t CORRELATION_WINDOW_MS = 150;  // max request->response latency we will pair (observed real latency 14-43ms)
constexpr uint32_t DUPLICATE_SUPPRESS_MS = 5;    // identical back-to-back frame within this window is treated as a re-parse artifact, not a new transmission

struct Frame {
  uint8_t id = 0;
  uint8_t cmd = 0;
  uint8_t len = 0;
  std::vector<uint8_t> payload;
  uint8_t chk = 0;
  uint8_t calc = 0;
  bool valid = false;
  uint32_t timestamp_ms = 0;
  std::vector<uint8_t> raw;  // full frame incl. header + checksum byte, for dedupe comparison
};

// Byte-level frame synchronizer for ONE direction of traffic. Feed it bytes
// one at a time, in arrival order, with a monotonic timestamp; it returns
// true and fills `out` whenever a complete frame (valid checksum or not --
// caller decides what to do with invalid ones) has just been assembled.
//
// Resync behaviour: any byte that is not a plausible header while waiting
// for one is silently dropped (this is how "unknown/garbage bytes between
// frames are safely ignored" is satisfied). A frame whose declared LEN byte
// is implausibly large is abandoned and the LEN byte itself is re-offered as
// a potential new header, rather than blocking forever waiting for payload
// bytes that funcitonally can't arrive correctly. A frame that is left
// half-received for longer than FRAME_TIMEOUT_MS is abandoned the same way
// (this is "recovery after interrupted serial data").
class FrameSync {
 public:
  bool feed(uint8_t byte, uint32_t now_ms, Frame &out);

 private:
  enum class State {
    WAIT_HEADER,
    WAIT_LEADIN_CONFIRM,
    READ_ID,
    READ_CMD,
    READ_LEN,
    READ_PAYLOAD,
    READ_CHK
  };
  State state_ = State::WAIT_HEADER;
  std::vector<uint8_t> buf_;
  uint32_t frame_start_ms_ = 0;

  void reset();
};

enum class RegTier { CONFIRMED, MATCHED_TEMP, MATCHED_ENUM, UNCONFIRMED, UNDECODED };

struct RegInfo {
  uint16_t addr;
  RegTier tier;
  const char *name;
};

// Single source of truth for which registers are known and how confident we
// are in them -- shared between the test harness and the ESPHome wrapper so
// "what gets published, and at what confidence tier" cannot drift between
// what was tested and what runs on the device.
//
// Tiers, matching PROTOCOL.md / PROTOCOL.md:
//   CONFIRMED    -- independently confirmed on our own Velis 80 captures via a
//                   controlled experiment (0x2D79 / 0x9E71 DHW setpoint).
//   MATCHED_TEMP -- exact address match between Velis 80's poll table and the
//                   Velis Evo repo's register map; decoded with the same
//                   uint16-LE/divide-by-10 convention used by the CONFIRMED
//                   temperature fields (a reasonable extension of a confirmed
//                   convention, not independently re-proven address-by-address).
//   MATCHED_ENUM -- address presence confirmed on both products, but this
//                   investigation's own data explicitly found ECO/BOOST/power
//                   on-off semantics "NOT LOCATED, inconclusive"
//                   (PROTOCOL.md S7) -- published as RAW
//                   bytes only, never as a decoded boolean/enum.
//   UNCONFIRMED  -- present only in the Velis Evo repo's own documentation,
//                   never cross-checked against our own Velis 80 data at all.
//   UNDECODED    -- polled by both products but neither this investigation nor
//                   the Evo repo assigns it a meaning -- logged, never published.
extern const std::vector<RegInfo> kKnownRegisters;
const RegInfo *lookup_register(uint16_t addr);

// Owns two independent FrameSync instances (one per physical CN6 wire) plus
// the request/response correlation and register-decode logic. No UART
// transmission anywhere in this class -- see file header.
class PassiveMonitor {
 public:
  // Fired for EVERY completed frame on each stream, valid or not -- useful
  // for logging/diagnostics and for the test harness's regression checks.
  std::function<void(const Frame &)> on_request_frame;
  std::function<void(const Frame &)> on_response_frame;

  // Fired only for frames this code can actually decode and that pass
  // dedupe (i.e. the value genuinely changed since last published).
  std::function<void(uint16_t addr, double value_c, const char *tier)> on_decoded_temperature;
  std::function<void(uint16_t addr, const std::vector<uint8_t> &raw, const char *tier)> on_decoded_raw;
  std::function<void()> on_unmatched_response;  // a response arrived with no pending request to pair it with, or too late

  // `request` = CN6 pin carrying gateway->MCU traffic (the existing
  // factory gateway's own commands -- id 0x41 in our capture data).
  // `response` = CN6 pin carrying MCU->gateway traffic (id 0x14).
  // Both are fed from receive-only taps; this class has no path back out.
  void feed_request_byte(uint8_t b, uint32_t now_ms);
  void feed_response_byte(uint8_t b, uint32_t now_ms);

  size_t request_frames_seen() const { return request_frames_seen_; }
  size_t request_frames_valid() const { return request_frames_valid_; }
  size_t response_frames_seen() const { return response_frames_seen_; }
  size_t response_frames_valid() const { return response_frames_valid_; }
  size_t correlated_pairs() const { return correlated_pairs_; }

 private:
  FrameSync request_sync_;
  FrameSync response_sync_;

  struct PendingRequest {
    bool active = false;
    uint8_t cmd = 0;
    std::vector<uint16_t> addresses;
    uint32_t timestamp_ms = 0;
  };
  PendingRequest pending_;

  std::vector<uint8_t> last_request_raw_;
  uint32_t last_request_ms_ = 0;
  std::vector<uint8_t> last_response_raw_;
  uint32_t last_response_ms_ = 0;

  std::map<uint16_t, uint16_t> last_temp_raw_;
  std::map<uint16_t, std::vector<uint8_t>> last_enum_raw_;

  size_t request_frames_seen_ = 0, request_frames_valid_ = 0;
  size_t response_frames_seen_ = 0, response_frames_valid_ = 0;
  size_t correlated_pairs_ = 0;

  void handle_request_frame(const Frame &f);
  void handle_response_frame(const Frame &f);
  void publish_decoded(uint16_t addr, const std::vector<uint8_t> &value_bytes);
};

}  // namespace ariston_passive
